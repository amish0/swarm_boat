from __future__ import annotations

import atexit
import json
import logging
import os
import secrets
import sqlite3
import threading
from datetime import datetime, timedelta, timezone
from functools import wraps
from pathlib import Path

import paho.mqtt.client as mqtt
from dotenv import load_dotenv
from flask import Flask, Response, flash, g, jsonify, redirect, render_template, request, session, url_for
from flask_limiter import Limiter
from flask_limiter.util import get_remote_address
from flask_wtf.csrf import CSRFProtect, generate_csrf
from werkzeug.security import check_password_hash
from werkzeug.middleware.proxy_fix import ProxyFix


PACKAGE_ROOT = Path(__file__).resolve().parent
ROOT = PACKAGE_ROOT.parent
load_dotenv(ROOT / ".env")


class TelemetryStore:
    def __init__(self, path: str, max_rows: int) -> None:
        self.path = Path(path).expanduser().resolve()
        self.max_rows = max(1000, max_rows)
        self.lock = threading.Lock()
        self.path.parent.mkdir(parents=True, exist_ok=True)
        with self._connect() as connection:
            connection.execute("PRAGMA journal_mode=WAL")
            connection.execute(
                """CREATE TABLE IF NOT EXISTS telemetry (
                    id INTEGER PRIMARY KEY AUTOINCREMENT,
                    received_at TEXT NOT NULL,
                    topic TEXT NOT NULL,
                    payload TEXT NOT NULL
                )"""
            )
            connection.execute("CREATE INDEX IF NOT EXISTS idx_telemetry_received ON telemetry(received_at)")

    def _connect(self) -> sqlite3.Connection:
        connection = sqlite3.connect(self.path, timeout=5)
        connection.row_factory = sqlite3.Row
        return connection

    def add(self, topic: str, payload: bytes | str) -> int:
        raw = payload.decode("utf-8", errors="replace") if isinstance(payload, bytes) else payload
        received_at = datetime.now(timezone.utc).isoformat(timespec="milliseconds")
        with self.lock, self._connect() as connection:
            cursor = connection.execute(
                "INSERT INTO telemetry (received_at, topic, payload) VALUES (?, ?, ?)",
                (received_at, topic, raw),
            )
            message_id = int(cursor.lastrowid)
            if message_id % 100 == 0:
                connection.execute(
                    "DELETE FROM telemetry WHERE id <= "
                    "(SELECT id FROM telemetry ORDER BY id DESC LIMIT 1 OFFSET ?)",
                    (self.max_rows,),
                )
        return message_id

    def messages_after(self, after_id: int, limit: int) -> list[dict]:
        if after_id == 0:
            query = "SELECT id, received_at, topic, payload FROM telemetry ORDER BY id DESC LIMIT ?"
            params = (limit,)
        else:
            query = "SELECT id, received_at, topic, payload FROM telemetry WHERE id > ? ORDER BY id ASC LIMIT ?"
            params = (after_id, limit)
        with self.lock, self._connect() as connection:
            rows = connection.execute(query, params).fetchall()
        messages = []
        for row in rows:
            raw = row["payload"]
            try:
                payload = json.loads(raw)
            except (TypeError, json.JSONDecodeError):
                payload = {"raw": raw}
            messages.append({
                "id": row["id"],
                "receivedAt": row["received_at"],
                "topic": row["topic"],
                "payload": payload,
                "raw": raw,
            })
        return messages


class MqttIngestor:
    def __init__(self, app: Flask, store: TelemetryStore) -> None:
        self.app = app
        self.store = store
        self.lock = threading.Lock()
        self.connected = False
        self.last_error = ""
        self.client: mqtt.Client | None = None
        self.configured = bool(app.config["MQTT_HOST"])
        self.topic = app.config["MQTT_TOPIC"]

    def start(self) -> None:
        if not self.configured:
            self.last_error = "MQTT_HOST is not configured"
            self.app.logger.warning("MQTT ingestion is disabled: MQTT_HOST is not configured")
            return

        client_id = self.app.config["MQTT_CLIENT_ID"] or f"ai-boat-monitor-{secrets.token_hex(4)}"
        client = mqtt.Client(
            callback_api_version=mqtt.CallbackAPIVersion.VERSION2,
            client_id=client_id,
            protocol=mqtt.MQTTv311,
        )
        username = self.app.config["MQTT_USERNAME"]
        if username:
            client.username_pw_set(username, self.app.config["MQTT_PASSWORD"])
        if self.app.config["MQTT_TLS"]:
            client.tls_set()

        def on_connect(mqtt_client, _userdata, _flags, reason_code, _properties) -> None:
            if reason_code.is_failure:
                with self.lock:
                    self.connected = False
                    self.last_error = f"Broker rejected connection ({reason_code})"
                return
            result, _mid = mqtt_client.subscribe(self.topic, qos=0)
            with self.lock:
                self.connected = result == mqtt.MQTT_ERR_SUCCESS
                self.last_error = "" if self.connected else "Broker subscription failed"

        def on_disconnect(_mqtt_client, _userdata, _disconnect_flags, reason_code, _properties) -> None:
            with self.lock:
                self.connected = False
                self.last_error = "Broker connection lost; reconnecting" if reason_code.is_failure else ""

        def on_message(_mqtt_client, _userdata, message) -> None:
            if len(message.payload) > self.app.config["MQTT_MAX_PAYLOAD_BYTES"]:
                self.app.logger.warning("Discarded oversized MQTT message on %s", message.topic)
                return
            try:
                self.store.add(message.topic, message.payload)
            except (OSError, sqlite3.Error):
                self.app.logger.exception("Unable to persist incoming MQTT telemetry")

        client.on_connect = on_connect
        client.on_disconnect = on_disconnect
        client.on_message = on_message
        self.client = client
        try:
            client.connect_async(
                self.app.config["MQTT_HOST"],
                self.app.config["MQTT_PORT"],
                keepalive=30,
            )
            client.loop_start()
        except (OSError, ValueError) as error:
            with self.lock:
                self.connected = False
                self.last_error = "Unable to start MQTT connection"
            self.app.logger.error("Unable to start MQTT client: %s", error)

    def stop(self) -> None:
        if self.client is not None:
            self.client.loop_stop()
            self.client.disconnect()

    def status(self) -> dict:
        with self.lock:
            return {
                "configured": self.configured,
                "connected": self.connected,
                "topic": self.topic,
                "error": self.last_error,
            }


def create_app(test_config: dict | None = None) -> Flask:
    app = Flask(__name__, instance_relative_config=True)
    environment = os.getenv("APP_ENV", "development").lower()
    secret_key = os.getenv("APP_SECRET_KEY")
    app.config.from_mapping(
        APP_ENV=environment,
        SECRET_KEY=secret_key or secrets.token_hex(32),
        APP_USERNAME=os.getenv("APP_USERNAME", ""),
        APP_PASSWORD_HASH=os.getenv("APP_PASSWORD_HASH", ""),
        SESSION_COOKIE_HTTPONLY=True,
        SESSION_COOKIE_SECURE=environment == "production",
        SESSION_COOKIE_SAMESITE="Strict",
        PERMANENT_SESSION_LIFETIME=timedelta(hours=8),
        WTF_CSRF_TIME_LIMIT=3600,
        RATELIMIT_STORAGE_URI=os.getenv("RATELIMIT_STORAGE_URI", "memory://"),
        RATELIMIT_ENABLED=True,
        RATELIMIT_HEADERS_ENABLED=True,
        MQTT_HOST=os.getenv("MQTT_HOST", ""),
        MQTT_PORT=int(os.getenv("MQTT_PORT", "8883")),
        MQTT_USERNAME=os.getenv("MQTT_USERNAME", ""),
        MQTT_PASSWORD=os.getenv("MQTT_PASSWORD", ""),
        MQTT_TOPIC=os.getenv("MQTT_TOPIC", "esp32/sensors"),
        MQTT_CLIENT_ID=os.getenv("MQTT_CLIENT_ID", ""),
        MQTT_TLS=os.getenv("MQTT_TLS", "true").lower() not in {"0", "false", "no"},
        MQTT_MAX_PAYLOAD_BYTES=int(os.getenv("MQTT_MAX_PAYLOAD_BYTES", "65536")),
        TELEMETRY_DB_PATH=os.getenv("TELEMETRY_DB_PATH", str(Path(app.instance_path) / "telemetry.sqlite3")),
        TELEMETRY_MAX_ROWS=int(os.getenv("TELEMETRY_MAX_ROWS", "100000")),
        TRUST_PROXY=os.getenv("TRUST_PROXY", "false").lower() in {"1", "true", "yes"},
        MAX_CONTENT_LENGTH=16 * 1024,
    )
    if test_config:
        app.config.update(test_config)

    if app.config["APP_ENV"] == "production" and not secret_key and not (test_config and test_config.get("SECRET_KEY")):
        raise RuntimeError("APP_SECRET_KEY must be configured in production")
    if not app.config["APP_USERNAME"] or not app.config["APP_PASSWORD_HASH"]:
        raise RuntimeError("Set APP_USERNAME and APP_PASSWORD_HASH in .env before starting the monitor")

    if app.config["TRUST_PROXY"]:
        app.wsgi_app = ProxyFix(app.wsgi_app, x_for=1, x_proto=1, x_host=1)

    csrf = CSRFProtect()
    csrf.init_app(app)
    limiter = Limiter(
        key_func=get_remote_address,
        default_limits=[],
        storage_uri=app.config["RATELIMIT_STORAGE_URI"],
        headers_enabled=app.config["RATELIMIT_HEADERS_ENABLED"],
    )
    limiter.init_app(app)
    app.extensions["monitoring_limiter"] = limiter

    store = TelemetryStore(app.config["TELEMETRY_DB_PATH"], app.config["TELEMETRY_MAX_ROWS"])
    ingestor = MqttIngestor(app, store)
    app.extensions["telemetry_store"] = store
    app.extensions["mqtt_ingestor"] = ingestor
    if not app.config.get("TESTING"):
        ingestor.start()
        atexit.register(ingestor.stop)

    @app.before_request
    def prepare_response_nonce() -> None:
        g.csp_nonce = secrets.token_urlsafe(18)

    @app.after_request
    def add_security_headers(response: Response) -> Response:
        response.headers["Content-Security-Policy"] = (
            "default-src 'self'; "
            f"script-src 'self' 'nonce-{g.csp_nonce}'; "
            "style-src 'self' 'unsafe-inline' https://fonts.googleapis.com; "
            "font-src 'self' https://fonts.gstatic.com; "
            "connect-src 'self'; img-src 'self' data:; "
            "base-uri 'self'; form-action 'self'; frame-ancestors 'none'"
        )
        response.headers["X-Content-Type-Options"] = "nosniff"
        response.headers["X-Frame-Options"] = "DENY"
        response.headers["Referrer-Policy"] = "strict-origin-when-cross-origin"
        response.headers["Permissions-Policy"] = "camera=(), microphone=(), geolocation=()"
        response.headers["Cache-Control"] = "no-store"
        if request.is_secure and app.config["APP_ENV"] == "production":
            response.headers["Strict-Transport-Security"] = "max-age=31536000; includeSubDomains"
        return response

    def authenticated(view):
        @wraps(view)
        def wrapped(*args, **kwargs):
            if not session.get("username"):
                if request.path.startswith("/api/"):
                    return jsonify(error="Authentication required"), 401
                return redirect(url_for("login", next=request.path))
            return view(*args, **kwargs)
        return wrapped

    @app.get("/healthz")
    def healthz():
        return jsonify(status="ok")

    @app.route("/login", methods=["GET", "POST"])
    @limiter.limit("5 per minute", methods=["POST"])
    def login():
        if session.get("username"):
            return redirect(url_for("dashboard"))
        if request.method == "POST":
            username = request.form.get("username", "")
            password = request.form.get("password", "")
            username_ok = secrets.compare_digest(username, app.config["APP_USERNAME"])
            try:
                password_ok = check_password_hash(app.config["APP_PASSWORD_HASH"], password)
            except (ValueError, TypeError):
                password_ok = False
            if username_ok and password_ok:
                session.clear()
                session.permanent = True
                session["username"] = app.config["APP_USERNAME"]
                destination = request.args.get("next", "")
                if not destination.startswith("/") or destination.startswith("//"):
                    destination = url_for("dashboard")
                return redirect(destination)
            flash("The username or password was not accepted.", "error")
        return render_template("login.html", username=app.config["APP_USERNAME"])

    @app.post("/logout")
    @authenticated
    def logout():
        session.clear()
        return redirect(url_for("login"))

    @app.get("/")
    @authenticated
    def dashboard():
        html_path = PACKAGE_ROOT / "static" / "mqtt-monitor.html"
        content = html_path.read_text(encoding="utf-8")
        content = content.replace('__CSP_NONCE__', g.csp_nonce).replace('__CSRF_TOKEN__', generate_csrf())
        return Response(content, mimetype="text/html")

    @app.get("/api/telemetry")
    @authenticated
    @limiter.limit("120 per minute")
    def telemetry():
        raw_after = request.args.get("after", "0")
        try:
            after_id = max(0, int(raw_after))
            limit = min(500, max(1, int(request.args.get("limit", "250"))))
        except ValueError:
            return jsonify(error="after and limit must be integers"), 400
        return jsonify(
            messages=store.messages_after(after_id, limit),
            status=ingestor.status(),
            serverTime=datetime.now(timezone.utc).isoformat(timespec="seconds"),
        )

    logging.basicConfig(level=os.getenv("LOG_LEVEL", "INFO").upper())
    return app


if __name__ == "__main__":
    application = create_app()
    application.run(host="127.0.0.1", port=int(os.getenv("PORT", "5000")), debug=False)
import re

from werkzeug.security import generate_password_hash

from monitoring import create_app


def make_app(tmp_path):
    return create_app({
        "TESTING": True,
        "SECRET_KEY": "test-session-secret",
        "APP_ENV": "testing",
        "APP_USERNAME": "operator",
        "APP_PASSWORD_HASH": generate_password_hash("a-long-test-password-123"),
        "MQTT_HOST": "",
        "RATELIMIT_ENABLED": False,
        "TELEMETRY_DB_PATH": str(tmp_path / "telemetry.sqlite3"),
    })


def csrf_token(response):
    match = re.search(rb'type="hidden" name="csrf_token" value="([^"]+)"', response.data)
    assert match
    return match.group(1).decode()


def test_dashboard_and_api_require_login(tmp_path):
    client = make_app(tmp_path).test_client()

    assert client.get("/").status_code == 302
    assert client.get("/api/telemetry").status_code == 401
    assert client.get("/healthz").json == {"status": "ok"}


def test_login_requires_csrf_and_accepts_configured_password(tmp_path):
    client = make_app(tmp_path).test_client()
    login_page = client.get("/login")
    token = csrf_token(login_page)

    rejected = client.post("/login", data={"username": "operator", "password": "wrong", "csrf_token": token})
    assert b"was not accepted" in rejected.data

    token = csrf_token(client.get("/login"))
    accepted = client.post(
        "/login",
        data={"username": "operator", "password": "a-long-test-password-123", "csrf_token": token},
    )
    assert accepted.status_code == 302
    assert client.get("/").status_code == 200


def test_authenticated_api_reads_persisted_telemetry(tmp_path):
    app = make_app(tmp_path)
    app.extensions["telemetry_store"].add(
        "esp32/sensors",
        b'{"event":"periodic","distanceCm":84,"temperatureC":24.6,"humidityPercent":58}',
    )
    client = app.test_client()
    token = csrf_token(client.get("/login"))
    client.post(
        "/login",
        data={"username": "operator", "password": "a-long-test-password-123", "csrf_token": token},
    )

    response = client.get("/api/telemetry")
    assert response.status_code == 200
    assert response.json["messages"][0]["payload"]["distanceCm"] == 84
    assert response.json["status"]["configured"] is False
    dashboard = client.get("/")
    html = dashboard.get_data(as_text=True)
    assert "__CSRF_TOKEN__" not in html
    assert "__CSP_NONCE__" not in html
    assert "unpkg.com/mqtt" not in html
    assert "broker-secret" not in html
    assert "nonce-" in dashboard.headers["Content-Security-Policy"]
    assert dashboard.headers["X-Frame-Options"] == "DENY"


def test_logout_requires_csrf_and_clears_session(tmp_path):
    client = make_app(tmp_path).test_client()
    token = csrf_token(client.get("/login"))
    client.post(
        "/login",
        data={"username": "operator", "password": "a-long-test-password-123", "csrf_token": token},
    )
    dashboard = client.get("/")
    token_match = re.search(rb'name="csrf_token" value="([^"]+)"', dashboard.data)
    assert token_match

    response = client.post("/logout", data={"csrf_token": token_match.group(1).decode()})
    assert response.status_code == 302
    assert client.get("/api/telemetry").status_code == 401
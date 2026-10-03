import getpass
import secrets

from werkzeug.security import generate_password_hash


def main() -> None:
    password = getpass.getpass("New monitor password: ")
    confirmation = getpass.getpass("Confirm password: ")
    if len(password) < 14:
        raise SystemExit("Use at least 14 characters for the monitor password.")
    if not secrets.compare_digest(password, confirmation):
        raise SystemExit("Passwords did not match.")
    print(generate_password_hash(password, method="scrypt"))


if __name__ == "__main__":
    main()
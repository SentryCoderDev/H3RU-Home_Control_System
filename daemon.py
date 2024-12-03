import os
import time
import subprocess

def is_process_running(process_name):
    """Belirtilen process'in çalışıp çalışmadığını kontrol eder."""
    try:
        subprocess.check_output(["pgrep", "-f", process_name])
        return True
    except subprocess.CalledProcessError:
        return False

def start_application():
    """Uygulamayı başlatır."""
    subprocess.Popen(["uvicorn", "app:app", "--host", "0.0.0.0", "--port", "8001"])

def main():
    process_name = "uvicorn"
    while True:
        if not is_process_running(process_name):
            print("Uygulama çalışmıyor, başlatılıyor...")
            start_application()
        else:
            print("Uygulama çalışıyor.")
        time.sleep(5)

if __name__ == "__main__":
    main()

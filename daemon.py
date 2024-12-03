import subprocess
import time
import psutil

def start_application():
    """Start the application."""
    subprocess.Popen(["/bin/bash", "-c", "cd /home/jetson/Desktop/H3RU/ && Xvfb :99 -screen 0 1024x768x16 & export DISPLAY=:99 && sudo python3 /home/jetson/Desktop/H3RU/app.py"])

def is_application_running():
    """Check if the application is running."""
    for proc in psutil.process_iter(['pid', 'name', 'cmdline']):
        if 'python3' in proc.info['cmdline'] and 'app.py' in proc.info['cmdline']:
            return True
    return False

if __name__ == "__main__":
    start_application()
    while True:
        time.sleep(600)  # Sleep for 10 minutes
        if not is_application_running():
            start_application()
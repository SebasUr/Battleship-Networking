# logging_utils.py

from datetime import datetime
from . import config

def log_transaction(query, response_id, client_id, server_response):
    now = datetime.now()
    date_str = now.strftime("%Y-%m-%d")
    time_str = now.strftime("%H:%M:%S")
    log_message = f"{date_str} {time_str} {client_id} {query} {response_id} {server_response}"
    with open(config.LOGFILE, "a") as log_file:
        log_file.write(log_message + "\n")


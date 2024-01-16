from datetime import datetime

def get_current_time():
    # Get the current date and time
    current_time = datetime.now()

    # Format the current time as a string
    formatted_time = current_time.strftime("%Y-%m-%d %H:%M:%S")

    return formatted_time

if __name__ == "__main__":
    current_time = get_current_time()
    print(f"Current Time: {current_time}")

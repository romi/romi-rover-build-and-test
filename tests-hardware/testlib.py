import json

def load_configuration_file(filename):
    print("Loading configuration")
    with open(filename, "r") as read_file:
        return json.load(read_file)

def request_confirmation(message):
    response = input(message + " [Y/n] ")
    if (response == "n"):
        raise AssertionError("User responded 'no'")

def ask(message):
    response = input(message + " [Y/n] ")
    return response == "" or response == "y"

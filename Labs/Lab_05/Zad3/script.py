import os
import sys


def test_zad3(N, producers_number, size, consumers_number, name):
    # Filling
    ins = {}

    for file_name in os.listdir(f'res/{name}/in'):
        with open(f'res/{name}/in/{file_name}', 'r') as file:
            line = file.readline()
            ins[file_name[0]] = [len(line), line[0]]
    c = 0
    for file_name in os.listdir(f'res/{name}/out'):

        with open(f'res/{name}/out/{file_name}', 'r') as file:
            for index, line in enumerate(file):
                for char in line:
                    index = str(index)
                    if index in ins.keys() and char == ins[index][1]:
                        ins[index][0] -= 1
    is_legit = True
    for value in ins.values():
        if value[0] != 0:
            is_legit = False
            break

    testing_types = {
        'many_to_many': ["Many producers are sending data to many consumers",
                         f"\n\tNumber of chars to send: {N}\n\tNumber of producers: {producers_number}\n\tSize of single string in line: {size}\n\tNumber of consumers: {consumers_number}"],
        'many_to_one': ["One producer is sending data to many consumers",
                        f"\n\tNumber of chars to send: {N}\n\tNumber of producers: {producers_number}\n\tSize of single string in line: {size}\n\tNumber of consumers: {consumers_number}"],
        'one_to_many': ["Many producers are sending data to one consumer",
                        f"\n\tNumber of chars to send: {N}\n\tNumber of producers: {producers_number}\n\tSize of single string in line: {size}\n\tNumber of consumers: {consumers_number}"]

    }
    print(
        "------------------------------------------Python Testing Script-------------------------------------------\n\n")
    if is_legit:

        print(f"| Type of testing: {name}\n")
        print(f"| Description: ", end="")
        print(f"{testing_types[name][0]}\n")
        print(f"| Parameters: {testing_types[name][1]}\n")
        print("_" * 102)
        print("\n")
        print(
            "------------------------------------------All Tests Passed!-------------------------------------------\n\n")
    else:
        print("\n------------------------------------------Test Failed!-------------------------------------------\n\n")


if __name__ == "__main__":
    test_zad3(*sys.argv[1:])

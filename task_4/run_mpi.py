import subprocess
import random


def generate_mpifile():
    coords = []
    for x in range(0, 4):
        for y in range(0, 8):
            for z in range(0, 8):
                for t in range(0, 2):
                    coords.append(str(x) + " " + str(y) + " " + str(z) + " " + str(t) + "\n")

    random.shuffle(coords)

    file = open('map.txt', 'w')
    for i in range(0, 512):
        file.write(coords[i])


def run_process(command):
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
    output = process.communicate()


def create_batch(m, n):
    print("./generator " + str(m) + " " + str(n))

    if m == 512 and n == 512:
            print("mpisubmit.bg -n 1 -w 00:05:00 --stdout " + str(m) + "_" + str(n) + "_custom.csv ./main.out -mapfile map.txt")
            run_process("mpisubmit.bg -n 1 -w 00:05:00 --stdout " + str(m) + "_" + str(n) + "_custom.csv ./main.out -mapfile map.txt")

    print("mpisubmit.bg -n 1 -w 00:05:00 --stdout " + str(m) + "_" + str(n) + ".csv ./main.out")
    run_process("mpisubmit.bg -n 1 -w 00:05:00 --stdout " + str(m) + "_" + str(n) + ".csv ./main.out")

    for process_pow in range(5, 10):
        run_process("./generator " + str(m) + " " + str(n))
        if m == 512 and n == 512:
            print("mpisubmit.bg -n " + str(2**process_pow) + " -w 00:05:00 --stdout " + str(m) + "_" + str(n) + "_custom.csv ./main.out -mapfile map.txt")
            run_process("mpisubmit.bg -n " + str(2**process_pow) + " -w 00:05:00 --stdout " + str(m) + "_" + str(n) + "_custom.csv ./main.out -mapfile map.txt")

        print("mpisubmit.bg -n " + str(2**process_pow) + " -w 00:05:00 --stdout " + str(m) + "_" + str(n) + ".csv ./main.out")
        run_process("mpisubmit.bg -n " + str(2**process_pow) + " -w 00:05:00 --stdout " + str(m) + "_" + str(n) + ".csv ./main.out")


generate_mpifile()

create_batch(512, 512)
create_batch(1024, 1024)
create_batch(2048, 2048)
create_batch(4096, 4096)
create_batch(4096, 1024)
create_batch(1024, 4096)
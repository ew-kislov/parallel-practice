import subprocess
import random


def generate_mpifile():
    coords = []
    for x in range(0, 4):
        for y in range(0, 8):
            for z in range(0, 8):
                for t in range(0, 2):
                    coords.append(str(x) + " " + str(y) + " " + str(z) + " " + str(t))

    random.shuffle(coords)

    file = open('map.txt', 'w')
    for i in range(0, 512):
        file.write(coords[i])


def run_process(command):
    process = subprocess.Popen(command.split(), stdout=subprocess.PIPE)
    output = process.communicate()


def create_batch(m, n):
    for process_pow in range(5, 10):
        run_process("python generate_matrix.py " + str(m) + " " + str(n))
        if m == 512 and n == 512:
            run_process("mpisubmit.bg -n " + 2**process_pow + "-w 00:05:00 --stdout " + str(m) + "_" + str(n) + "_custom.csv ./main.out -mapfile map.txt")

        run_process("mpisubmit.bg -n " + 2**process_pow + "-w 00:05:00 --stdout " + str(m) + "_" + str(n) + ".csv ./main.out")


generate_mpifile()

create_batch(512, 512)
create_batch(1024, 1024)
create_batch(2048, 2048)
create_batch(4096, 4096)
create_batch(4096, 1024)
create_batch(1024, 4096)
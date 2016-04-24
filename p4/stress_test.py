import os
import random
import sys
from termcolor import colored
import time

def check():
    for i in xrange(int(sys.argv[1])):
        print "----------------------------------------"
        #file_size = random.randint(0, int(sys.argv[2]))
        file_size = 1024 * 1024 * 1024 * 2
        print colored("FILE SIZE:", "blue"), colored(str(file_size), "cyan")
        st = time.time()
        #os.system("python test4.py --file_sz {0} --dcnt 100000 --endian little".format(file_size))
        os.system("python test4.py --file_sz 200 --wcnt 20 --dcnt 5 --endian little --seed 12345")
        #os.system("python test4.py --endian little --file_sz 104857600 --seed 12345 --wcnt 40000 --dcnt 1000")
        print colored("CREATED TEST FILE", "yellow", attrs=["blink"])
        print colored("TIME: {0}".format(time.time() - st), "magenta", attrs=["blink"])

        st = time.time()
        os.system("./reverse_index test")
        print colored("RUN MY PROGRAM", "yellow", attrs=["blink"])
        print colored("TIME: {0}".format(time.time() - st), "magenta", attrs=["blink"])
        
        st = time.time()
        os.system("./checker test")
        print colored("RUN CHECKER", "yellow", attrs=["blink"])
        print colored("TIME: {0}".format(time.time() - st), "magenta", attrs=["blink"])       
        
        os.system("cmp reverse_index.bin right.bin \n echo $? > res")
        
        f = open("res", "r")
        res = int(f.readline().strip())
        if res == 0:
            print colored("OK", "green")
        else:
            print colored("FAIL", "red")
            sys.exit(0)
    
check()

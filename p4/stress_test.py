import os
import random
import sys
from termcolor import colored
import time

def check():
    for i in xrange(1000):
        print "----------------------------------------"
        file_size = random.randint(0, 1024 * 100)
        print colored("FILE SIZE:", "blue"), colored(str(file_size), "cyan")
        st = time.time()
        os.system("python test4.py --file_sz {0} --endian little".format(file_size))
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
        
        os.system("cmp reverse_index.bin right.bin")
        os.system("echo $? > res")
        
        f = open("res", "r")
        res = int(f.readline().strip())
        if res == 0:
            print colored("OK", "green")
        else:
            print colored("FAIL", "red")
            sys.exit(0)
    
check()
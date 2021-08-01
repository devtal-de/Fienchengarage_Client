#!/usr/bin/python

import sys, getopt, datetime, math
from hashlib import sha256

def main(argv):
    shared_secret = ""
    id = ""
    pin = ""
    tuer = ""
    date = ""
    try:
        opts, args = getopt.getopt(argv,"s:i:p:t:d:")
        if len(opts) != 5:
            print("gen_access_code.py -s <shared_secret> -i <cardid> -p <pin> -t <tuer> -d <dayoffset>")
            sys.exit(2)
    except getopt.GetoptError:
        print("gen_access_code.py -s <shared_secret> -i <cardid> -p <pin> -t <tuer> -d <dayoffset>")
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print("gen_access_code.py -s <shared_secret> -i <cardid> -p <pin> -t <tuer> -d <dayoffset>")
            sys.exit()
        elif opt == "-s":
            shared_secret = str(arg)
        elif opt == "-i":
            id = arg
        elif opt == "-p":
            pin = arg
        elif opt  == "-t":
            tuer = str(arg)
        elif opt == "-d":
            date = (datetime.datetime.today() + datetime.timedelta(days=int(arg))).strftime("%Y%m%d")
            #print(str(date))
 
    print( sha256((shared_secret+id+pin+date+tuer).encode("utf-8")).hexdigest() )


if __name__ == "__main__":
   main(sys.argv[1:])

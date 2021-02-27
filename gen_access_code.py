#!/usr/bin/python

import sys, getopt, datetime, math
from hashlib import sha256

def main(argv):
    id = ""
    pin = ""
    tuer = ""
    date = ""
    try:
        opts, args = getopt.getopt(argv,":iptd:")
        if len(args) != 7:
            print("gen_access_code.py -i <cardid> -p <pin> -t <tuer> -d <dayoffset>")
            sys.exit(2)
    except getopt.GetoptError:
        print("gen_access_code.py -i <cardid> -p <pin> -t <tuer> -d <dayoffset>")
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print("gen_access_code.py -i <cardid> -p <pin> -t <tuer> -d <dayoffset>")
            sys.exit()
        elif opt in ("-i"):
            id = arg
        elif opt in ("-p"):
            pin = arg
        elif opt in ("-t"):
            tuer = str(arg)
        elif opt in ("-d"):
            date = str( math.floor((datetime.datetime.today() + datetime.timedelta(days=int(arg))).timestamp() / 60 / 60 / 24)* 60 * 60 * 24 )
            #print(str(date))
 
    print( sha256((id+pin+date+tuer).encode("utf-8")).hexdigest() )


if __name__ == "__main__":
   main(sys.argv[1:])

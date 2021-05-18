import os
import sys

import numpy as np
import pylab as plt

from PIL import Image
from matplotlib import cm


def load_bin(fname):
    f = open(fname, 'rb')
    
    arr = []
    tmp = []

    while True:
        data = f.read(2)

        if data == b'':
            break

        val = int.from_bytes(data, byteorder='little')

        tmp.append(val)
        if len(tmp) == 19200:
            arr.append(np.array(tmp).reshape((120, 160)))
            tmp = []

    f.close()

    return arr
   

def writepng_pil(im, ofname):
    vmin = np.min(im)
    vmax = np.max(im)
    im = np.array((im - 26315) / 3000 * 255, dtype=np.uint8)
    arr = cm.rainbow(im)[:,:,:3]*255
    arr = np.array(arr, dtype=np.uint8)
    img = Image.fromarray(arr, 'RGB')
    img.save(ofname)


def arr2png(arr, fname, outpath='./png'):
    for i, im in enumerate(arr):
        ofname = f'{outpath}/{fname[-4:]}-overwrite.png'
        writepng_pil(im, ofname)


def convert_dir(path):
    flist = os.listdir(path)
    flist.sort()
    for fname in flist:
        infile = os.path.join(path, fname)
        print (infile)
        convert_bin(infile)


def convert_bin(fname):
    arr = load_bin(fname)
    ofname = fname.split('/')[-1] 
    arr2png(arr, ofname)


def main():
    arg = sys.argv[1]

    if os.path.isdir(arg):
        path = arg
        convert_dir(path)
    else:
        fname = arg
        convert_bin(fname)


if __name__=='__main__':
    main()


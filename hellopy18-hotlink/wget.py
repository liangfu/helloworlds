#!/usr/bin/env python

# download resource with hotlinking with python requests

import shutil
import requests
import sys,os
from urlparse import urlsplit

def wget(url):
    referer = '://'.join(urlsplit(url)[0:2])
    filename = os.path.basename(url)
    headers = {
        'User-Agent': 'Mozilla/5.0 (Macintosh; Intel Mac OS X 10_10_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/39.0.2171.95 Safari/537.36',
        'Referer' : referer+'/',
    }
    r = requests.get(url,headers=headers,stream=True)
    with open(filename, 'wb') as f:
        shutil.copyfileobj(r.raw, f)
    print 'finish downloading', filename

if __name__=='__main__':
    wget(sys.argv[1])

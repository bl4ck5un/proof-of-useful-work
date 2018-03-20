#!/usr/bin/python
# Copyright 2012-2014 Luke Dashjr
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the standard MIT license.  See COPYING for more details.

from blkmaker import _dblsha256
import blktemplate
import json
import struct
import sys

# This test_input data is released under the terms of the Creative Commons "CC0 1.0 Universal" license and/or copyright waiver.
# test_input = '''
# {
# 	"result": {
# 		"previousblockhash": "000000004d424dec1c660a68456b8271d09628a80cc62583e5904f5894a2483c",
# 		"target": "00000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffff",
# 		"noncerange": "00000000ffffffff",
# 		"transactions": [],
# 		"sigoplimit": 20000,
# 		"expires": 120,
# 		"longpoll": "/LP",
# 		"height": 23957,
# 		"coinbasetxn": {
# 			"data": "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff1302955d0f00456c6967697573005047dc66085fffffffff02fff1052a010000001976a9144ebeb1cd26d6227635828d60d3e0ed7d0da248fb88ac01000000000000001976a9147c866aee1fa2f3b3d5effad576df3dbf1f07475588ac00000000"
# 		},
# 		"version": 2,
# 		"curtime": 1346886758,
# 		"mutable": ["coinbase/append"],
# 		"sizelimit": 1000000,
# 		"bits": "ffff001d"
# 	},
# 	"id": 0,
# 	"error": null
# }
# '''

test_input = '''
{
	"result": {
  "capabilities": [
    "proposal"
  ],
  "version": 536870913,
  "previousblockhash": "fe1b97906bd161cbd0705c9489a0d55c1898f0c3905a24666c2246246de891d7",
  "transactions": [
    {
      "data": "01000000019fb0f4539d73c710064dd4ba5262e04f6b263985fc854d2c88ce7197262c21210000000048473044022021b39b95ab26536e60718a58c2fbe9161afc0c3fe9327eda22fef4742d01c2f502204168b1a2412cf9f05920b140ba90feb75506efc3f735979f54f055ccc18c280101feffffff020065cd1d000000001976a91461fb3cad045d64bc756558e1cb040e0baeb5e03a88ac007e380c010000001976a9142eace074cfdc4fe58e06f54e2fc82937a1d4ef6b88ac9f020000",
      "hash": "e7c70b5ccced688f2553f96c15623e3a37bb5d4a60991a40b5fe86abf3cc6e90",
      "depends": [
      ],
      "fee": 3840,
      "sigops": 2
    }
  ],
  "coinbaseaux": {
    "flags": ""
  },
  "coinbasevalue": 5000003840,
"coinbasetxn": {
   "data": "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff1302955d0f00456c6967697573005047dc66085fffffffff02fff1052a010000001976a9144ebeb1cd26d6227635828d60d3e0ed7d0da248fb88ac01000000000000001976a9147c866aee1fa2f3b3d5effad576df3dbf1f07475588ac00000000"
   },
  "longpollid": "fe1b97906bd161cbd0705c9489a0d55c1898f0c3905a24666c2246246de891d7675",
  "target": "000000000000803f000000000000000000000000000000000000000000000000",
  "mintime": 1476810724,
  "mutable": [
    "time",
    "transactions",
    "prevblock"
  ],
  "noncerange": "00000000ffffffff",
  "sigoplimit": 20000,
  "sizelimit": 1000000,
  "curtime": 1476816413,
  "bits": "1b00803f",
  "height": 672
	},
	"id": 0,
	"error": null
}
'''


def send_json(req):
	print(json.dumps(req, indent=4))

tmpl = blktemplate.Template()
req = tmpl.request()

# send req to server and parse response into req
send_json(req)
if (len(sys.argv) == 2):
	req = json.load(sys.stdin)
else:
	req = json.loads(test_input)
	send_json(req)

try:
	# Bypass Python 3 idiocy
	range = xrange
except:
	pass

tmpl.add(req)

(data, dataid) = tmpl.get_data()
assert(len(data) >= 76)

data = data[:76]
blkhash = _dblsha256(data)
print("Block hash prefix for pouw: " + blkhash)
# TODO: impl
print("Block difficulty for pouw: " + "TODO")


while (tmpl.time_left() and tmpl.work_left()):
	(data, dataid) = tmpl.get_data()
	assert(len(data) >= 76)

 	# mine the right nonce
 	for nonce in range(0x7fffffff):
 		data = data[:76] + struct.pack('!I', nonce)
 		blkhash = _dblsha256(data)
 		if blkhash[28:] == b'\0\0\0\0':
 			break
 		if (not (nonce % 0x1000)):
 			sys.stdout.write("0x%8x hashes done...\r" % nonce)
 			sys.stdout.flush()
 	print("Found nonce: 0x%8x \n" % nonce)

 	req = tmpl.submit(data, dataid, nonce)
 	# send req to server
 	send_json(req) 

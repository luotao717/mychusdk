
import re
import os
import collections


def load(datpath):
	print("load(%s)"%(datpath))
	with open(datpath, "r") as fp:
		data = fp.read();
		pattern = re.compile("(\w+)?=")
		result=pattern.findall(data)
		print(len(result))
		#print(result)
		datlist=set(result)
		print(len(datlist))
		print("duplicate", 
			[x for x, y in collections.Counter(datlist).items() if y > 1])
	return datlist


def diff(path1, path2):
	print("diff(%s, %s)"%(os.path.basename(path1), os.path.basename(path2)))
	dat1=[]
	dat2=[]
	dat1=load(path1)
	dat2=load(path2)
	for each in dat1:
		if each not in dat2:
			print("missing %s"%each)
		else:
			#print("=== %s"%each)
			pass
base="uci2dat.dat"
path1="package/ralink/drivers/mt7620/files/mt7620.dat"
path2="package/ralink/drivers/mt76x2e/files/mt7612.dat"

print("--------------")
diff(path1, base)
print("--------------")
diff(path2, base)
load(base)

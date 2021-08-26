import sys

file1 = sys.argv[1]
file2 = "Assignment/2_" + sys.argv[1]

f1 = open(file1, "r")
f2 = open(file2, "r")

print(f1.read() == f2.read()[::-1])

f1.close()
f2.close()

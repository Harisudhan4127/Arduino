import os
import time as t
print("Welcome")
t.sleep(2)
# print("Current Time : ", t.time())
# print("Local time : ", t.localtime())
print(t.strftime("Date : %d-%m-%Y time : %H:%M:%S", t.localtime()))
# os.system("cls")
import sys

file_name = sys.argv[1]
for line in open(file_name):
 if "irit.color(" in line:
   cur_line = line
   length = len(cur_line)
   cur_line = cur_line[:length-3]   # Now last chat in line is color number.
   #print cur_line
   color_num = cur_line[ len(cur_line)-2:]
   color_num = int(color_num)
   if (color_num == 0):
     cur_line = cur_line[:len(cur_line)-1] + "irit.BLACK )"
   elif (color_num == 1):
     cur_line = cur_line[:len(cur_line)-1] + "irit.BLUE )"
   elif (color_num == 2):
     cur_line = cur_line[:len(cur_line)-1] + "irit.GREEN )"
   elif (color_num == 3):
     cur_line = cur_line[:len(cur_line)-1] + "irit.CYAN )"
   elif (color_num == 4):
     cur_line = cur_line[:len(cur_line)-1] + "irit.RED )"
   elif (color_num == 5):
     cur_line = cur_line[:len(cur_line)-1] + "irit.MAGENTA )"
   elif (color_num == 6):
     cur_line = cur_line[:len(cur_line)-1] + "irit.YELLOW )"
   elif (color_num == 7):
     cur_line = cur_line[:len(cur_line)-1] + "irit.WHITE )"
   elif (color_num == 14):
     cur_line = cur_line[:len(cur_line)-1] + "irit.YELLOW )"
   elif (color_num == 15):
     cur_line = cur_line[:len(cur_line)-1] + "irit.WHITE )"
   else:
     print color_num
     
   #print cur_line
   #print "\n"

# hed
Simple hex editor    
ESC - normal mode  
s - HEX mode    
a - ASCII mode  
i[s/a] - (HEX/ASCII) Insert mode  
In normal mode:  
h, j, k, l - left, down, up, right  
q - quit  
w - save    
@[offset in hex] - jump to offset  
/[word] - search after cursor and jump to first match  
?[word] - search before cursor and jump to first match  
gg - jump to start of file  
G - jump to end of file  
$ - jump to end of line  
x - delete byte  
b - add zero byte  
In other modes:  
Backspace - return to previous byte  
u - undo last change  
r - cancel undo :/

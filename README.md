# CustomConsoleOutput
### A dll injected into process to control every line of output to console as you want.

You just need to modify Customize.cpp and recompile it to get the working dll.  

---

In&nbsp;&nbsp;```Customize.cpp```&nbsp;&nbsp;you can find only one function:  
```bool OutputProcess(std::string& line)```  

Every time when a line is to output, this function will be called to process, check or modify it, even to stop it from output when necessary.

---

Add your code in that function to process the output.  
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;return false - if you don't want this line to be output  
(Note that string "line" is passed by quote)

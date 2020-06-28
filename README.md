# Arithm
A basic arithmetic function parser GUI based on Arash Partow's *exprtk*. If you're not fond of the usual OS calculators for your daily routine, this application can be used for calculations of arithmetic expressions. It can also plot functions.

<p align="center">
  <img src="https://github.com/bmartensen/Arithm/blob/master/media/Arithm.png" alt="Screenshot">
</p>

# Usage

Apart from entering arithmetic expressions, the plot intervals *\[x_min, x_max\]* and *\[y_min, y_max\]* can be set during runtime using the *:=* operator. The default horizontal plot interval can be configured via *Arithm.ini*. The variable *x* is reserved for evaluating the expressions for plotting.

The user functions *f*, *g* and *h* can be explicitly defined. Please note that no plot will be displayed if neither user functions nor variables are used. In this case, only a constant result value is displayed (calculator function).

The most recent expression is automatically saved on application exit and is available via selection on the next application start. This feature can be configured and turned on/off via settings in *Arithm.ini*.

# Syntax

See https://github.com/bmartensen/Arithm/blob/master/exprtk for available functions and expressions.

# License

See https://github.com/bmartensen/Arithm/blob/master/LICENSE for details.

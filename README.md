# Arithm
A basic arithmetic function parser GUI based on the exprtk. If you're not fond of the usual OS calculators for your daily routine, this application can be used for calculations of common arithmetic expressions. It can also plot functions.

<p align="center">
  <img src="https://github.com/bmartensen/Arithm/blob/master/media/Arithm_Multiplot.png" alt="Screenshot">
</p>

# Usage

Apart entering from arithmetic expressions, the variables *a* (plot start), *b* (plot end) and *i* (plot samples) can be set during runtime or can be predefined via *Arithm.ini*. The variable *x* is reserved for evaluating the expression in a plot.

The functions *f*, *g* and *h* can be explicitely defined. Please note that no plot is generated if the expression result over the defined interval *\[a, b\]* is constant. In this case, only the constant value is displayed (calculator function).

# Syntax

See <a href="https://github.com/bmartensen/Arithm/blob/master/exprtk" target="_blank">https://github.com/bmartensen/Arithm/blob/master/exprtk</a> for available functions and expressions.

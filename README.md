# Arithm
A basic arithmetic function parser GUI based on Arash Partow's *exprtk*. If you're not fond of the usual OS calculators for your daily routine, this application can be used for calculations of arithmetic expressions. It can also plot functions.

<p align="center">
  <img src="https://github.com/bmartensen/Arithm/blob/master/media/Arithm.png" alt="Screenshot">
</p>

# Usage

Apart entering from arithmetic expressions, the variables *a* (plot start), *b* (plot end) and *i* (plot samples) can be set during runtime or can be defined via *Arithm.ini*. The variable *x* is reserved for evaluating the expressions for plotting.

The functions *f*, *g* and *h* can be explicitely defined. Please note that no plot is generated if the expression result is constant over the defined interval *\[a, b\]*. In this case, only the constant value is displayed (calculator function).

# Syntax

See https://github.com/bmartensen/Arithm/blob/master/exprtk for available functions and expressions.

# License

See https://github.com/bmartensen/Arithm/blob/master/LICENSE for details.

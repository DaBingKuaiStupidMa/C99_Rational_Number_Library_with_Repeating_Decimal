中文用户可以方便地阅读代码注释，因为大冰块的母语是中文。This lib uses C language to implement calculation, comparison, output, input of rational numbers (fractions and decimals), and the transformation between native C data types and rational numbers, within limited precision, repeating decimal supported. Here's the so-called "rational class" :
```
typedef struct rat{
    rint up; // Naming conflicts avoided
    urint down: sizeof(urint)*8-1;
    unsigned splfd: 1;
}rat; // Rint defined by macro
```
The default precision is long long integer if you don't do anything, but
you can decide which kind of int to use in "rat" by these operations:
```
#define _RAT_USE_INT_
#define _RAT_USE_LONG_
```
This library is strictly designed to produce _mathematically fit_ results under _finite precision_. I/O functions only use `fwrite()`, `fgetc()` and `ungetc()` to work. The library supports a wide variety of input formats:
- Integers: `42`, `-7`
- Fractions: `2/3`, `-2/-3`, `/23`, `23/`
- Decimals: `0.23`, `.23`, `-23.`
- Repeating decimals: `1.2(3)`, `-.(142857)`

Omissions around `.` are treated as `0`, while other ones around `/` are treated as `1`, and the `(3)` in `0.2(3)` denotes the repeating period. Here are some of the library's most important functions:
```
rat RatSimp(rat); // 约分 (Simplify)
rat RatPlus(rat,rat);
rat RatTimes(rat,rat);
signed char RatCmp(rat,rat); // 比较 (Comparison) ">"1, "=="0, "<"-1, "NaN"2.

char PutRat(rat); // 分数输出 (Fraction Style)
char PutDecimal(rat,short); // 按n位小数输出 (Decimal Style with Requested Precision)
rint PutRepeat(rat); // 小数形式输出直到循环节结束 (Repeating Decimal)

rat GetRat(); // 综合输入（整数、分数、小数）
// Global Input Function Supporting Int, Fraction, and Decimal Style at the same time, Repeating Decimal Included.

rat LdbToRat(long double);  // long double 化为 rat
// This function also works when there's precision-related overflow.
```
RatTest File is provided, and there's a micro example:
```
#include "rational.h"
int main(){
    rat a, b, c;
    a=rONE; // a defined const
    b=GetRat(); // You could enter "-.0(143857)" for instance.
    c=RatPlus(a,b);
    PutRat(c);
    return 0;
}
```
In case of you don't know:
- You could test your PC's performance by PutRepeat((rat){1,2147483647,1}); .
- Sorry the code comments in the files are in Chinese, which describes what each function and design is made for.
- Use as you want as long as saving the code comment which shows the lib's author name at the beginning of .h.
- English Version of .txt with TRANSLATED code comments would be ready in the future.

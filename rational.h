#ifndef _RATIONAL_H_BY_DBK_
#define _RATIONAL_H_BY_DBK_
#define _DBK_RATIONAL_VERSION_ 202605010L
/*  这个有理库在 *有限精度* 下实现有理数运算、输入、输出和转化等功
    能，支持 0 分母，支持循环小数输入输出。函数的行为见rational.c。

*   本库来自 @大冰块stupid吗，联系方式有
    腾讯QQ(推荐): 3287178592
    电话(不推荐): 13518828772
    dabingkuaistupidma@gmail.com*/

#include <stdio.h>
//  #define _RAT_USE_INT_
//  #define _RAT_USE_LONG_

#ifdef _RAT_USE_INT_
    #define rint int
    #define urint unsigned int
#elif defined _RAT_USE_LONG_
    #define rint long
    #define urint unsigned long
#else // Default Precision
    #define rint long long
    #define urint unsigned long long
#endif

    /*有理型*/
typedef struct rat{
    rint up;
    urint down: sizeof(urint)*8-1;
    unsigned splfd: 1;
}rat;
/*  三个成员分别是：分子、非负的分母、化简状态。

*   splfd (simplified) 的值为 1 时，认为一个rat已经化简过。本库
    *全自动* 管理化简状态。初始化时，假定您不知道分数是否最简，仅
    指定分子分母的值，让splfd被机械填 0 即可。化简一个分数之前，必
    须保证 splfd 成员的值为 0。

*   splfd 是本库一切函数（包括化简函数）判断分数是否最简的唯一依据。
    因此，您可以将 splfd 初始化为 1 来保护一个 不应该 被化简的有
    理值。

*   每一个成员变量的设计都有缘由。正负性完全由分子决定，分母总是正的，
    化简标记仅 1 位，这让分子分母范围能够一致。

*   库的实现中，涉及运算的整型全都写成(u)rint，衍生常量都按照以下基
    本常量来定义。也就是说，光是解除 #define _RAT_USE_..._的注释，
    就可以更改整个库的精度范围。*/

#include <limits.h>
#ifdef _RAT_USE_INT_
    #define RAT_MAX INT_MAX
    #define RAT_MIN INT_MIN
#elif defined _RAT_USE_LONG_
    #define RAT_MAX LONG_MAX
    #define RAT_MIN LONG_MIN
#else // Default Precision
    #define RAT_MAX LLONG_MAX
    #define RAT_MIN LLONG_MIN
#endif

#define _RAT_OVERFLOW_TO_NAN_
/*  溢出分为两类，一是精度溢出，这时算法会返回一个分子分母都在精度范
    围内的近似值，二是数值超出了能够表示的最大范围，这个版本的库默认
    认为须返回强报错 NaN (0/0)，而不是边界值如 RAT_MAX，也不是无
    穷值如 +inf (1/0)。这个规则在 *运算与输入* 一致。
*   如果需要保证运算或输入从不返回错误，将_RAT_OVERFLOW_NO_FAULT_
    的宏定义移除移，即可令数值溢出返回 RAT_MAX 或 RAT_MIN。*/

#if RAT_MAX==9223372036854775807LL /*Default Precision*/
    #define _D_L_O_R_ 19 /*_DECIMAL_LENGTH_OF_RAT_MAX_*/
    #define _SQRTRM_ 3037000499LL /*_SQRT_OF_RAT_MAX_*/
    #define RAT_SIZE 16
#elif RAT_MAX==2147483647L
    #define _D_L_O_R_ 10
    #define _SQRTRM_ 46340L
    #define RAT_SIZE 8
#elif RAT_MAX==32767 /*This is RARE.*/
    #define _D_L_O_R_ 5
    #define _SQRTRM_ 181
    #define RAT_SIZE 4
#endif // RAT_SIZE defined as the POSSIBLE SMALLEST case

/*  常量：0、1、分数式的正无穷、负无穷、不定态等。它们的值为：
    {0,1,1}, {1,1,1}, {1,0,1}, {-1,0,1}, {0,0,1}.
    推荐用rZERO初始化有理值。另外，一切零分母分数统称无值分数。*/
extern const rat rZERO, rONE, rINF, rNEGINF, rUNCERTAIN;
extern const rat rMAX, rMIN;

/*  这是一些特制常量，用于承载错误信息。除了人为初始化，它们仅有可能
    通过本库的输入函数获得。这些量被 splfd 保护，意味着错误信息要通
    过赋值来销毁。错误信息若流入运算函数（包括比较）或输出函数，直接
    视为UNCERTAIN(NaN)。它们的分母均为 0，splfd均为 1，分子从 2
    起 +1 递增。*/
extern const rat _getrat_irrelated_; // 直接遇到无关字符。
extern const rat _getrat_met_eof_; // 直接遇到EOF。
extern const rat _getrat_null_char_; // 直接遇到'\0'空符。

rat RatSimp(rat);       // 约分
rat RatPlus(rat,rat);   // 加法
rat RatNeg(rat);        // 求负
rat RatMinus(rat,rat);  // 减法
rat RatTimes(rat,rat);  // 乘法
rat RatFlip(rat);       // 倒数
rat RatDivi(rat,rat);   // 除法
rat RatCalc(rat,char,rat);  // 二元运算包，char取编程四则运算符
rat RatAbs(rat);            // 绝对值
signed char RatCmp(rat,rat);// 比较大小 ">"1, "=="0, "<"-1, "NaN"2.

#define _DOB_SIZE_ 512 // Decimal output buffer's size

char PutRat(rat);                       // 分数输出
unsigned short PutDecimal(rat,short);   // 按n位小数输出（禁四舍五入）
rint PutRepeat(rat);                    // 小数形式输出直到循环节结束
rat GetRat();                           // 综合输入（整数、分数、小数）

long double RatToLdb(rat);  // rat 化为 long double
double RatToDb(rat);        // rat 化为 double
rat LdbToRat(long double);  // long double 化为 rat
rint RatToRint(rat);        // 化为整数

//  File IO Func

char fPutRat(rat,FILE *);
unsigned short fPutDecimal(rat,short,FILE *);
rint fPutRepeat(rat,FILE *);
rat fGetRat(FILE *);

//  String IO Func

/*  String GetRat
    Recommended Method:
    char *temp=str;
    rat result=sGetRat( & temp,size);
    int consumed=(int)(temp-str);
    char NEVER_DO_THIS =*temp;*/
rat sGetRat(char **AddrOfPtr,unsigned short size);
char sPutRat(rat q,char *str,unsigned short size);
unsigned short sPutDecimal(rat q,short n,char *str,unsigned short size);
// undefined sPutRepeat(rat NaN,char *NULL,unsigned short SlZE);

/*  Binarily Serialize a rat and return 1;
    The size MUST be the same as RAT_SIZE, if not, return 0;*/
char RatToBin(rat q,unsigned char *s,char size);
/*  Do the opposite. The size MUST be the same as RAT_SIZE,
    if not, return rUNCERTAIN;*/
rat BinToRat(unsigned char *s,char size);

//  end of rational.h
#endif

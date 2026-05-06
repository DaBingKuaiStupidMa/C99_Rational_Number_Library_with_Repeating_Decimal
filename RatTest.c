#define __USE_MINGW_ANSI_STDIO 1
// #define _RAT_USE_INT_
#include <stdio.h>
#include "rational.h"
#define g GetRat()
#define n putchar('\n')
//  这程序测试rational中各个功能。
int main(){
    rat a,b,c; int opt; long double f;
    while(opt=0, 1){
        puts("测试项目：\n\
1.加法  2.减法  3.乘法  4.除法\n\
5.化简  6.负数  7.倒数  8.输入\n\
9.分数输出 10.n位小数输出 11.准确小数输出\n\
12.rat转Z   13.rat转flt  14.比较大小\n\
15.二元运算 16.flt转rat 17.穷举计时");
        scanf("%d",&opt);
        switch(opt){
            case 1: a=g; b=g;
                c=RatPlus(a,b);
                PutRat(c);
                n; printf("by float: %.20Lf",(RatToLdb(a)+RatToLdb(b)));
                n; printf("by rat: %.20Lf",(RatToLdb(c)));
                break;
            case 2: a=g; b=g;
                c=RatMinus(a,b);
                PutRat(c);
                n; printf("by float: %.20Lf",(RatToLdb(a)-RatToLdb(b)));
                n; printf("by rat: %.20Lf",(RatToLdb(c)));
                break;
            case 3: a=g; b=g;
                c=RatTimes(a,b);
                PutRat(c);
                n; printf("by float: %.20Lf",(RatToLdb(a)*RatToLdb(b)));
                n; printf("by rat: %.20Lf",(RatToLdb(c)));
                break;
            case 4: a=g; b=g;
                c=RatDivi(a,b);
                PutRat(c);
                n; printf("by float: %.20Lf",(RatToLdb(a)/RatToLdb(b)));
                n; printf("by rat: %.20Lf",(RatToLdb(c)));
                break;
            case 5: a=g; PutRat(RatSimp(a)); break;
            case 6: a=g; PutRat(RatNeg(a)); break;
            case 7: a=g; PutRat(RatFlip(a)); break;
            case 8:
            case 9: puts("用英文括号标出循环节（如有）");
                a=g; printf("\n输出%d个字符。",PutRat(a));
                n; printf("to float: %.20Lf",(RatToLdb(a)));
                break;
            case 10: a=g; printf("\n非小数部分%d个字符",PutDecimal(a,g.up));
                n; printf("by float: %.20Lf",(RatToLdb(a)));
                break;
            case 11: a=g; printf("\n循环节长%d位",PutRepeat(a));
                n; printf("by float: %.20Lf",(RatToLdb(a)));
                break;
            case 12: a=g; printf("%lld",RatToRint(a)); break;
            case 13: a=g; printf("%lf",RatToLdb(a)); break;
            case 14: a=g; b=g; printf("%d",RatCmp(a,b)); break;
            case 15: puts("输入示范：\"-2.4+-7.9\" \"3/2/6/5\" \"0.(428571)*2.(3)\" \".-/\"");
                a=g; opt=getchar(); b=g; PutRat(RatCalc(a,opt,b)); break;
            case 16:
                scanf("%Lf",&f); a=LdbToRat(f); PutRat(a);
                puts(" 即"); PutDecimal(a,20); break;
            case 17:
                fputs("穷举内容详见RatTest.c文件，请手工计时，按Enter开始。",stdout);
                while(getchar()!='\n');
                for(int i=0; i<128; ++i)
                    for(int j=0; j<i; ++j)
                        for(int k=0; k<128; ++k)
                            for(int l=0; l<k; ++l){
                                RatPlus((rat){j,i,0},(rat){l,k,0});
                                RatTimes((rat){j,i,0},(rat){l,k,0});
                            }break;
            default: goto end;
        }   while(getchar()!='\n'); n;
    }end:;
}
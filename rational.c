#include "rational.h"
//  #define _INTRO_TRS_
/*  Introduce Typical Rational Strings: 本功能由上方的宏
    定义开启，旨在为特定的字符串匹配输入结果。比如"+inf"这样的
    输入，就会被解析为rINF，当然也可以用1/0表示。*/

const rat
rZERO={0,1,1},    rONE={1,1,1}, rINF={1,0,1},
rNEGINF={-1,0,1}, rUNCERTAIN={0,0,1};

#ifdef _INTRO_TRS_
/*  为了保证安全，指定的特殊字符串和配对的特殊数值以常量形式储存，
    这使得注册新的TRS必须手动修改代码。
*   个性化添加示范：
    假定您要添加一个叫作"ratmin"的代号，匹配{RAT_MIN,1,1}，那
    么您需要将_NOTRS_的值改为4，将_RIB_SIZE_的值改为6，在TRS
    数组后添上 "ratmin"，在TRSL数组后添上6，在TLR数组后添上 {
    LR_MIN,1,1}，不需要修改其他任何地方。*/
#define _NOTRS_ 3       // The number of Typical Rat Strings.
#define _RIB_SIZE_ 4    // Rat input buffer's size SHOULDE be the max length of TRS.
const char *_T_RS_[_NOTRS_] = {"+inf","-inf","NaN"};        /* typical rat strings */
const char  _T_RSL_[_NOTRS_]= {4,4,3};                      /* typical rat string lengths */
const lr    _T_LR_[_NOTRS_] = {{1,0,1},{-1,0,1},{0,0,1}};   /* typical longrats */
/*  借助一个极小的 buffer，判断是否正在输入 TRS。若是，返回匹
    配的有理值，若不是，将 buffer 中的所有字符 归还 到输入流。
*   库的作者在自己的电脑上使用此功能没有任何问题。如果发现与un-
    getc函数有关的bug，请不要_INTRO_TRS_。*/
    /*判断是否输入了特殊字符串。*/
lr _trs_judge_(FILE *fp){
    char _R_IB_[_RIB_SIZE_]={0}; /* rat input buffer */
    char _W_SF_[_NOTRS_]   ={0}; /* whether string fits (0->Yes 1->No) */
    char n=0,i;
    while(n<_RIB_SIZE_){
        _R_IB_[n]=fgetc(fp);
        for(i=0;i<_NOTRS_;++i){ // 遍历_typical_rat_strings_.
            if(_R_IB_[n]!=_T_RS_[i][n]) // 任一位字符不相同，做标记。
                _W_SF_[i]=1;
            if(!_W_SF_[i] && n+1==_T_RSL_[i]) // 没被标记过且达到长度，返回相应值。
                return _T_LR_[i];
        }
        for(i=0;i<_NOTRS_;++i) // 检查_whether_string_fits_.
            if(!_W_SF_[i]) break;
        if(i==_NOTRS_)
            {++n;break;}
        else ++n;
    }
    while(n>0)
        ungetc(_R_IB_[--n],fp);
    return rZERO;
}
#endif

/*  rational 作为一个有限的精度类型，所表示数集的元素在数轴上离
    散地分布，且越靠近零点越密集。易知最小的正 rat 为 1/RAT_MAX,
    对于区间 (RAT_MIN,RAT_MAX) 内的实数，总能找到一个 rat 型
    的近似值，简单估计发现相对误差不应超过 2/RAT_MAX */

//  因此设专门的小量，方便运算中的决策。辅助宏总是在文件结尾销毁。
#define ldb long double
const ldb _rtdt_=1/(ldb)RAT_MAX; // delta of rat
#define _BETW_RTDT_(r) (r<=_rtdt_ && r>=-_rtdt_)
const ldb _rtep_=2/(ldb)RAT_MAX; // epsilon of rat
#define _BETW_RTEP_(r) (r<=_rtep_ && r>=-_rtep_)

//  阻止编译器高优化导致溢出检查被删。
#ifdef __GNUC__
    #pragma GCC push_options
    #pragma GCC optimize("O0")
#elif defined(_MSC_VER)
    #pragma optimize("", off)
#endif
//  _MULTIPLICATION_FAILED_JUDGEMENTS_，乘法正确性校验，抗溢出。
//  用于无符号乘法。第一、第二条件阻止 0 乘法进入后续逻辑。
static char _MUL_FAIL1_(urint prod, urint fac1, urint fac2){
    return (fac1 && fac2 && (prod%fac1 || prod%fac2 || prod>RAT_MAX));
}// 检查最小公分母。因prod有符号，加入第三条件针对RAT_MAX+1。
static char _MUL_FAIL2_(rint prod, urint fac1, urint fac2){
    return (prod%(rint)fac1 || prod%(rint)fac2 || prod<0);
}// 检查通分后分子。当分子恰为RAT_MAX+1，乘法失败，当分子恰为RAT_MIN，乘法成功。
static char _MUL_FAIL3_(rint prod, rint fac1, urint fac2){
    return (prod%fac1 || prod%(rint)fac2 || ((fac1<0)!=(prod<0)));
}// 检查分子相乘，当乘积恰为RAT_MAX+1，乘法失败，当乘积恰为RAT_MIN，乘法成功。
static char _MUL_FAIL4_(rint prod, rint fac1, rint fac2){
    return (prod%fac1 || prod%fac2 || ((fac1<0)==(fac2<0) && prod<0));
}// 保护阈结束。
#ifdef __GNUC__
    #pragma GCC pop_options
#elif defined(_MSC_VER)
    #pragma optimize("", on)
#endif

//  这个经典的算法完全由 袁同学 介绍，在此感谢！
//  参数的精度是根据有效数字（相对误差）划定的。
    /* long double 化为 rat */
rat LdbToRat(ldb r){
    if(r>RAT_MAX-1 || r<RAT_MIN+1)
        return (rat){r>=0? RAT_MAX:RAT_MIN,1,1};
    if(_BETW_RTDT_(r))
        return rZERO;
    char minus=(r<0);
    if(minus)r=-r;
    urint Ax=1,Ay=0,Bx=0,By=1;
    urint up,down,k,prod;
    ldb value,temp;
    while(1){
        up=Ay+By; down=Ax+Bx;
        value=(ldb)up/down;
        temp=(value-r)/r;
        if(_BETW_RTEP_(temp))
            break;
        if(value<r){
            if(_BETW_RTEP_(By/(ldb)Bx-r))
                return (rat)\
                {minus ? -(rint)By:By,Bx,1};
            else k=(r*Ax-Ay)/(By-r*Bx);
            prod=k*Bx;
            if(_MUL_FAIL1_(prod,k,Bx)) break;
            Ax+=prod;
            if(Ax>RAT_MAX) break;
            prod=k*By;
            if(_MUL_FAIL1_(prod,k,By)) break;
            Ay+=prod;
            if(Ay>RAT_MAX) break;
        }
        else{
            if(_BETW_RTEP_(Ay/(ldb)Ax-r))
                return (rat)\
                {minus ? -(rint)Ay:Ay,Ax,1};
            else k=(r*Bx-By)/(Ay-r*Ax);
            prod=k*Ax;
            if(_MUL_FAIL1_(prod,k,Ax)) break;
            Bx+=prod;
            if(Bx>RAT_MAX) break;
            prod=k*Ay;
            if(_MUL_FAIL1_(prod,k,Ay)) break;
            By+=prod;
            if(By>RAT_MAX) break;
        }
    }return (rat){minus ? -(rint)up:up,down,1};
}

//  返回值非负，且有_rgcd_(0,n)==_rgcd_(n,0)==n。
    /*最大公约数*/
static inline rint _rgcd_(rint a,rint b){
    rint temp;
    while(b)
        temp=a%b,a=b,b=temp;
    return a<0 ? -a:a;
}

//  返回值非负，认为0与任何数的最小公倍数为0。
    /*最小公倍数*/
static inline urint _rlcm_(rint a,rint b){
    if(!a||!b) return 0;
    if(a<0) a=-a;
    if(b<0) b=-b;
    urint aa=a,bb=b,temp;
    while(bb)
        temp=aa%bb,aa=bb,bb=temp;
    return a/aa*b;
}

/*  做运算的有理数必须经过RatSimp检查，它使用形参，返回化简结果。
    这是本库 *唯一* 有权修改splfd成员的函数。*/
    /*约分*/
rat RatSimp(rat q){
//  无需化简，立即返回。
    if(q.splfd==1) return q;
    if(q.down==1) return q.splfd=1,q;
//  单独处理UNCERTAIN的情况。
    if(!q.up&&!q.down) return rUNCERTAIN;
//  正无穷和负无穷能够被归纳。
    rint temp=_rgcd_(q.up,q.down);
        q.up/=temp;
        q.down/=temp;
        q.splfd=1;
    return q;
}

//  为了阻止不必要的溢出校验，简单判断应无溢出的情况。
#define _BETW_SQRTRM_(q) (_SQRTRM_>=((q).up>0 ? (q).up:-(q).up) && _SQRTRM_>=(q).down)
#define _IS_SAFE_(x,y) (_BETW_SQRTRM_(x) && _BETW_SQRTRM_(y))

    /*加法*/
rat RatPlus(rat x,rat y){
    if(x.splfd!=1) x=RatSimp(x);
    if(y.splfd!=1) y=RatSimp(y);
//  无值分数专列。
    if(!x.down||!y.down){
        //销毁误入加法函数的输入错误信息。
        if(!x.down && x.up>1) x=rUNCERTAIN;
        if(!y.down && y.up>1) y=rUNCERTAIN;
        //只有一个无值分数则返回它。
        if(x.down) return y;
        if(y.down) return x;
        //认为同种无值分数之和不变。
        if(x.up==y.up) return x;
        //不同的无值分数和为不定态。
        else return rUNCERTAIN;
    }
//  其中一方为零，立即返回另一方。
    if(!x.up) return y;
    if(!y.up) return x;
//  加法的溢出来源有：数值溢出，精度溢出。
    ldb value=
        x.up/(ldb)x.down+
        y.up/(ldb)y.down;
    if(value>RAT_MAX-1 || value<RAT_MIN+1)
        return (rat){value>=0? RAT_MAX:RAT_MIN,1,1};
    if(_BETW_RTDT_(value))
        return rZERO;
    char safe=_IS_SAFE_(x,y);

    rint down=_rlcm_(x.down,y.down);
    if(!safe && _MUL_FAIL2_(down,x.down,y.down))
        return LdbToRat(value);

    urint temp=down/x.down;
    rint up=x.up*(rint)temp;
    if(!safe && _MUL_FAIL3_(up,x.up,temp))
        return LdbToRat(value);

    temp=down/y.down;
    up=y.up*(rint)temp;
    if(!safe && _MUL_FAIL3_(up,y.up,temp))
        return LdbToRat(value);

    if( ! (x.up>=0 ^ y.up>=0)){
        if(x.up>0) temp= down/x.down*x.up + down/y.down*y.up;
        else temp= down/x.down*(-x.up) + down/y.down*(-y.up);
        if(temp-(x.up<0) > RAT_MAX)
            return LdbToRat(value);
        x.up= x.up>=0 ? temp:-(rint)temp;
    }else x.up= (rint)(down/x.down)*x.up + (rint)(down/y.down)*y.up;

    x.down=down; x.splfd=0;
    return RatSimp(x);
}

    /*求负*/
rat RatNeg(rat q) {return q.up=(q.up==RAT_MIN ? RAT_MAX : -q.up),q;}

    /*减法*/
rat RatMinus(rat x,rat y) {y.up=(y.up==RAT_MIN ? RAT_MAX : -y.up); return RatPlus(x,y);}

    /*乘法*/
rat RatTimes(rat x,rat y){
    if(x.splfd!=1) x=RatSimp(x);
    if(y.splfd!=1) y=RatSimp(y);
//  无值分数专列。
    if(!x.down||!y.down){
//  销毁误入乘法函数的输入错误信息。
        if(!x.down && x.up>1) x=rUNCERTAIN;
        if(!y.down && y.up>1) y=rUNCERTAIN;
//  这三行涉及复杂的分类讨论。
        if(!x.up||!y.up) return rUNCERTAIN;
        if(x.up>0 ^ y.up>0) return rNEGINF;
        else return rINF; //同号得正，异号得负。
    }
//  乘法的溢出来源有：数值溢出，精度溢出。
    ldb value=
        (x.up/(ldb)x.down)*
        (y.up/(ldb)y.down);
    if(value>RAT_MAX-1 || value<RAT_MIN+1)
        return (rat){value>=0? RAT_MAX:RAT_MIN,1,1};
    if(_BETW_RTDT_(value))
        return rZERO;
//  单独处理整数乘法。
    if(x.down==1&&y.down==1) return x.up*=y.up,x;
    rint temp;
    if(temp=_rgcd_(x.up,y.down),temp!=1)   x.up/=temp,y.down/=temp;
    if(temp=_rgcd_(y.up,x.down),temp!=1)   y.up/=temp,x.down/=temp;
    char safe=_IS_SAFE_(x,y);

    temp=x.up*y.up;
    if(!safe && _MUL_FAIL4_(temp,x.up,y.up))
        return LdbToRat(value);
    temp=x.down*y.down;
    if(!safe && _MUL_FAIL2_(temp,x.down,y.down))
        return LdbToRat(value);

    x.up*=y.up;
    x.down=temp;
    return x;
}

//  认为无穷的倒数为0，0的倒数为正无穷，不定态的倒数为不定态。
    /*倒数*/
rat RatFlip(rat q){
    if(q.splfd!=1) q=RatSimp(q);
    char minus=0; rint temp;
//  鉴于分母类型为urint，引入minus记录正负性。
    if(q.up<0)
        q.up=(q.up==RAT_MIN ? RAT_MAX : -q.up),
        minus=1;
    temp=q.up;
    q.up=q.down;
    q.down=temp;
    if(minus) q.up=-q.up;
    return q;
}

    /*除法*/
rat RatDivi(rat x,rat y){
    char minus=0; rint temp;
    if(y.up<0)
        y.up = (y.up == RAT_MIN ? RAT_MAX : -y.up),
        minus = 1;
    temp=y.up;
    y.up=y.down;
    y.down=temp;
    if(minus) y.up=-y.up;
    return RatTimes(x,y);
}

    /*calc of 2 longrats 运算包（char取编程四则运算符）*/
rat RatCalc(rat x,char ch,rat y){
    switch(ch){
        case '+':return RatPlus(x,y);
        case '-':return RatMinus(x,y);
        case '*':return RatTimes(x,y);
        case '/':return RatDivi(x,y);
        default :return rUNCERTAIN;
    }
}

    /*rat 绝对值*/
rat RatAbs(rat q)
    {return q.up>=0 ? q:(q.up=(q.up==RAT_MIN ? RAT_MAX : -q.up),q);}

//  认为无穷大等于自身，但不定态 *不等于* 包括自身的任何值，\
    被比较的两数，只要出现 NaN 即返回 2。
    /*rat 比较大小 ">"1, "=="0, "<"-1, "NaN"2.*/
signed char RatCmp(rat x,rat y){
    if(x.splfd!=1) x=RatSimp(x);
    if(y.splfd!=1) y=RatSimp(y);
//  出现无值分数。
    if(!x.down||!y.down){
//  销毁误入比较函数的输入错误信息。
        if(!x.down && x.up>1) x=rUNCERTAIN;
        if(!y.down && y.up>1) y=rUNCERTAIN;
//  检查NaN是否出现。
        if(!x.up && !x.down) return 2;
        if(!y.up && !y.down) return 2;
//  若另一个是有值分数，返回相应的分子。
        if(x.down) return -y.up;
        if(y.down) return x.up;
//  正常的比较。
        if(x.up==y.up)
            return 0;
        return x.up>y.up ? 1:-1;
    }
//  剔除异号case。
    if((x.up>0)!=(y.up>0))
        return x.up>y.up ? 1:-1;
//  如果同分母，那太好了。
    if(x.down==y.down){
        if(x.up==y.up) return 0;
        else return x.up>y.up ? 1:-1;
    }
    char pozneg=x.up>0 ? 1:-1;
    urint temp1,temp2;
    if(pozneg>0) temp1=x.up/x.down, temp2=y.up/y.down;
    else temp1=-(x.up/x.down), temp2=-(y.up/y.down);
//  如果整数部分不相等，那太好了。
    if(temp1!=temp2)
        return temp1>temp2 ? 1*pozneg:-1*pozneg;

//  取余数，按 *二进制* 解析小数部分。
    if(pozneg>0) temp1=x.up%x.down, temp2=y.up%y.down;
    else temp1=-(x.up%x.down), temp2=-(y.up%y.down);
    char dgt;
//  dgt的第二位存放x小数部分的当前位，第一位存放y的。
    while(1){
        temp1<<=1;
        temp2<<=1;
        dgt=temp1/x.down; dgt<<=1;
        dgt+=temp2/y.down;
        if(dgt==2) return 1*pozneg;
        if(dgt==1) return -1*pozneg;
        temp1%=x.down;
        temp2%=y.down;
    }
}

    /*rat 化为 ldb*/
ldb RatToLdb(rat q)
    {return q.up/(ldb)q.down;}

    /*rat 化为 double*/
double RatToDb(rat q)
    {return q.up/(double)q.down;}

//  以rint边界代替所谓的无穷，其他无值分数对应0。
    /*rat 化为整数*/
rint RatToRint(rat q){
    if(q.splfd!=1) q=RatSimp(q);
    if(!q.down){
        switch(q.up){
            case 1: return RAT_MAX;
            case-1: return RAT_MIN;
            default:return 0;
        }
    }
    return q.up/(rint)q.down;
}

/*  本库输入函数意外终止时立即休克，直接返回休克时解析完成的rat。
*   除了小数输出返回循环节长度以外，一切输出函数返回（休克时的）
    偏移量变化值（极端情况下不准确）。
*   输入函数不反馈偏移量，您需要在读取前后分别使用ftell函数并作
    差，自行计算偏移量的变化，敬请谅解。
*   无论输入输出，您都需要自行判断文件是否到达了EOF等潜在问题。*/

//  这条宏 不 检查fputs是否返回了EOF，使得输出函数的返回值变得\
    并不准确。最好用ftell获知偏移量变化值。
#define _PUT_CHECK_1_(q){\
    if(!q.down){\
        switch(q.up){\
            case -1: fputs("-inf",fp); return 4;\
            case 1: fputs("+inf",fp); return 4;\
            default: fputs("NaN",fp); return 3;\
        }\
    }\
}
#define _PUTU_PPI_(n,i,buffer){\
    ten=1,lv=2,m=n;\
    while(n/=10) ten*=10,++lv;\
    while(--lv){\
        buffer[i]=m/ten+'0';\
        m%=ten;ten/=10; ++i;\
}}
//  以分数形式输出rat到FILE，返回输出字符个数。
    /* file PutRat */
char fPutRat(rat q,FILE *fp){
    if(q.splfd!=1) q=RatSimp(q);
//  拦截无值分数。
    _PUT_CHECK_1_(q)
    char _R_OB_[2*_D_L_O_R_+2]={0}; // rat output buffer
    char i=0;
    urint q_up;
    if(q.up<0){ ++i;
        _R_OB_[0]='-';
        q_up=-q.up;
    }else q_up=q.up;
    urint ten,lv,m;
    _PUTU_PPI_(q_up,i,_R_OB_)
    if(q.down==1) return fwrite(_R_OB_,1,i,fp);
//  分数线前后。
    _R_OB_[i++]='/';
    _PUTU_PPI_(q.down,i,_R_OB_)
    return fwrite(_R_OB_,1,i,fp);
}

    /*rat 分数输出*/
char PutRat(rat q) {return fPutRat(q,stdout);}

const rint _R_A_T_O_=RAT_MAX/10;// _RAT_ABOUT_TO_OVERFLOW_
const char _E_D_O_R_=RAT_MAX%10+'0';// _END_DIGIT_OF_RAT_MAX_

//  按n位小数输出rat到FILE，返回非小数部分（含.）字符数。
//  禁四舍五入，高于 32767 位的小数部分属于 *未定义行为*。
    /*File PutDecimal*/
char fPutDecimal(rat q,short n,FILE *fp){
    if(q.splfd!=1) q=RatSimp(q);
    _PUT_CHECK_1_(q)
    char _D_OB_[_DOB_SIZE_]={0};
    int i=0; char res=0;
    rint temp; urint q_up;
    if(q.up<0){
        _D_OB_[i++]='-';
        q_up=-q.up;
    }else q_up=q.up;
    urint ten,lv,m;
    if(q_up>=q.down){
        temp=q_up/q.down;
        _PUTU_PPI_(temp,i,_D_OB_)
        q_up%=q.down;
    }else _D_OB_[i++]='0';
//  与下方的--n呼应，提速并剔除负精度。
    if(++n==1)
        return fwrite(_D_OB_,1,i,fp);
    if(n<1) n=7; // 遇负精度，输出六位。
    _D_OB_[i++]='.'; res=i;

//  分子过大会因*10溢出，无法正确作商，故移位近似。
    if(q.down>_R_A_T_O_){
        q.up=q_up/=10;
        q.down/=10;
        q.splfd=0;
        q=RatSimp(q);
    }

    while(--n){
        q_up*=10;
        _D_OB_[i++]=q_up/q.down+'0';
        if(i==_DOB_SIZE_){
            fwrite(_D_OB_,1,i,fp);
            i=0;
        }q_up%=q.down;
    }fwrite(_D_OB_,1,i,fp);
    return res;
}

    /*按n位小数输出（禁四舍五入）*/
char PutDecimal(rat q,short n) {return fPutDecimal(q,n,stdout);}

#define _PUT_CHECK_2_(q){\
    if(!q.down){\
        switch(q.up){\
            case -1: fputs("-inf",fp); return 0;\
            case 1: fputs("+inf",fp); return 0;\
            default: fputs("NaN",fp); return 0;\
        }\
    }\
}//int lv=2;与--lv呼应。

//  准确地以小数形式输出。返回 *已输出* 的循环节长度。
//  如果分母过大等原因导致循环节不能正确输出，返回-1。
//  循环节的长度可以非常大，只受数据规模和EOF的限制。
    /* file PutRepeat */
rint fPutRepeat(rat q,FILE *fp){
    if(q.splfd!=1) q=RatSimp(q);
    if(q.down>_R_A_T_O_){
        fPutDecimal(q,20,fp);
        return -1;
    }_PUT_CHECK_2_(q)
    char _D_OB_[_DOB_SIZE_]={0};
    int i=0;
    rint temp; urint q_up;
    if(q.up<0){
        _D_OB_[i++]='-';
        q_up=-q.up;
    }else q_up=q.up;
    urint ten,lv,m;
    if(q_up>=q.down){
        temp=q_up/q.down;
        _PUTU_PPI_(temp,i,_D_OB_)
        q_up%=q.down;
    }else _D_OB_[i++]='0';

    _D_OB_[i++]='.';
    temp=q.down;
    rint n1=0,n2=0;
//  根据数论的结论，分母中包含因数5或2的个数，\
    决定了混循环小数的小数点与循环节间“废位”数。
    while(! (temp%5))   ++n1,temp/=5;
    while(! (temp%2))   ++n2,temp/=2;
    if(n2>n1) n1=n2;    //n1存放废位长度。

    short cnt=0;for(;cnt<n1;++cnt){
        q_up*=10;
        _D_OB_[i++]=q_up/q.down+'0';
        if(i==_DOB_SIZE_){
            fwrite(_D_OB_,1,i,fp);
            i=0;
        }q_up%=q.down;
    }if(!q_up && !n1) --i; // 整数不应输出小数点。
    fwrite(_D_OB_,1,i,fp); i=0;
    if(!q_up) return 0; // 发现是有限小数，截停。

    if(fputc('(',fp)<0) return -1;
    n1=q_up*10; n2=1;
    if(fputc(n1/q.down+'0',fp)<0) return n2;
    n1%=q.down;
//  q.up存放循环节首余数，n2记录循环节长度。
    while(n1!=q_up){
        _D_OB_[i++]=n1*10/q.down+'0';
        if(i==_DOB_SIZE_){
            cnt=fwrite(_D_OB_,1,i,fp);
            if(cnt<_DOB_SIZE_) return n2+cnt;
            i=0; n2+=_DOB_SIZE_;
        }n1=n1*10%q.down;
    }n2+=fwrite(_D_OB_,1,i,fp);
    fputc(')',fp);
    return n2;
}

    /*小数形式输出直到循环节结束*/
rint PutRepeat(rat q) {return fPutRepeat(q,stdout);}

const rat _getrat_irrelated_={2,0,1};
const rat _getrat_met_eof_={3,0,1};
const rat _getrat_null_char_={4,0,1};
#define _IS_BLANK_(ch) (ch==' '||ch=='\n'||ch=='\r'||ch=='\t')
#define _IS_DIGIT_(ch) (ch>='0'&&ch<='9')
    /*用temp添加数码到n的个位，如某次添加即将引起溢出，就冻结temp。*/
#define _ADD_DIGIT_(n,temp){\
    while(_IS_DIGIT_(temp)){\
        if(n>=_R_A_T_O_ && (n>_R_A_T_O_ || temp>_E_D_O_R_))\
            break;\
        n=n*10+(temp-'0');\
        temp=fgetc(fp);\
    }\
}
    /*输入小数部分时分母是10的幂，分母即将溢出时，把分子四舍五
    入并冻结temp。应当指出宏遇到(而退出时，一定没有发生溢出。*/
#define _POINT_ADD_DIGIT_(up,down,temp){\
    while(_IS_DIGIT_(temp)){\
        if(down>_R_A_T_O_){\
            if(temp>='5')\
                ++up;\
            break;\
        }\
        up=up*10+(temp-'0');\
        down*=10;\
        temp=fgetc(fp);\
    }\
}
    /*有的分数输入，分子分母奇大，但是数量级接近，所以理应有一
    个近似值，这时有必要统计分子分母相差多少（十进制）位。这里
    的宏作用在于统计（准）分子溢出RAT_MAX多少位。*/
#define _OVERFLOW_STATS_(count) while(_IS_DIGIT_(temp)) ++count,temp=fgetc(fp);
    /*整数部分已经溢出，则不解析小数部分，注意循环节也被跳过，
    但是如果输入形如LARGE.123(alpha，则(会被归还到输入流。*/
#define _SKIP_DECIMAL_PART_ {\
    do temp=fgetc(fp);\
    while(_IS_DIGIT_(temp));\
    if(temp=='('){\
        temp=fgetc(fp);\
        if(!_IS_DIGIT_(temp)){\
            ungetc(temp,fp);\
            ungetc('(',fp);\
        }\
        do temp=fgetc(fp);\
        while(_IS_DIGIT_(temp));\
        if(temp!=')')\
            ungetc(temp,fp);\
    }\
    else ungetc(temp,fp);\
}

/*  这个输入函数同时支持 整数、分数、小数 三种形式的有理数输入，
    输入小数时，您还可以用 *英文* 括号来标记循环节。当小数点前
    后发生缺失时，认为省略了 0，当分数线前后出现缺失时，认为省
    略了 1。

*   fGetRat将输入开头的所有空白字符跳过，包括换行，不包括'\0'。
    这意味着如果只输入一些空白而没有具体字符，fGetRat不仅不会
    归还空白，而且还会陷入停滞。数据开头最多有 1 个'+'或'-'，
    不然就不读取，返回{2,0,1}。

*   输入失败的情形：
    一切输入失败都是因为 没有 遇到任何 数字 引起的，包括直接遇
    到无关字符，直接遇到'\0'，直接遇到EOF。输入失败就返回特制的
    信息分数。
    const rat _getrat_irrelated_; // {2,0,1}
    const rat _getrat_met_eof_; // {3,0,1}
    const rat _getrat_null_char_; // {4,0,1}

*   输入结束的情形：
    解析了一些数字后，fGetRat可以因为空白字符自然结束，可能因为
    无关字符强行结束，或者遇到EOF而意外结束。无论以何种方式结束，
    fGetRat都 *不会* 吸收数字后继的任何字符（循环节的右括号当然
    是数字的一部分，把它除外）。输入函数意外终止时立即休克，直接
    返回休克时已经解析的数值。
        eg1: -2/-3会被解析为2/3。
        eg2: 2.3()和2.3(./a)的括号不会被侵吞。

*   设计上，fGetRat不借助buffer，直接逐字符解析。这种设计意味着
    它节省了空间，但是反复调用fgetc也带来了时间与性能开销，解析
    流程也错综复杂。此外，若输入流不允许归还字符，那么fGetRat将
    吞噬数字后至少 1 个无关字符。除了严重的屎山嫌疑，这种无法回头
    的设计至少一个问题：分数形式输入中，无法准确接收 RAT_MIN/n。

*   没有开启特殊字符串输入匹配的时候，fGetRat并不支持诸如"+inf"
    "-inf""NaN"等形式的输入。然而，无论在何种模式，您都能以分数
    形式"1/0""-1/0"和"0/0"代表它们。

*   fGetRat(fp)不支持反斜线和全角数字。*/
    /* file GetRat */
rat fGetRat(FILE *fp){
//  丢弃开头的空白字符，包括换行。
    rat q=rZERO;
    signed char temp;
    do temp=fgetc(fp);
    while(_IS_BLANK_(temp));

#ifdef _INTRO_TRS_
    ungetc(temp,fp);
//  处理特殊字符串。
    q=_trs_judge_(fp);
    if(q.up!=0 || q.down!=1)
        return q;
    temp=fgetc(fp);
#endif

//  either positive or negative. 
    char pozneg=1; int count=0;
//  常规输入开头预处理或分流。
    switch(temp){
        case '-': pozneg=-pozneg;
        case '+': temp=fgetc(fp);
            if(_IS_DIGIT_(temp)) break;
            if(temp!='/'&&temp!='.'){
                if(temp!=EOF)
                    ungetc(temp,fp);
                ungetc(pozneg>0?'+':'-',fp);
                return _getrat_irrelated_;
            }if(temp=='/') {q.up=1; goto _after_slash_;}
        case '.': goto _after_point_;
        case '/': q.up=1; goto _after_slash_;
        default :
            if(!_IS_DIGIT_(temp)){
                if(temp!=EOF)
                    ungetc(temp,fp);
                switch(temp){
                    case EOF: return _getrat_met_eof_;
                    case 0 : return _getrat_null_char_;
                    default: return _getrat_irrelated_;
                }
            }
    }

//  解析第一段数码。

    _ADD_DIGIT_(q.up,temp)
    if(_IS_DIGIT_(temp)){
        _OVERFLOW_STATS_(count)
        if(temp=='.'){
            _SKIP_DECIMAL_PART_
        }if(temp!='/'){
            ungetc(temp,fp);
            return (q.up=pozneg>0 ? RAT_MAX:RAT_MIN), q;
        }
    }if(temp!='/'&&temp!='.'){
        ungetc(temp,fp);
        if(pozneg<0) q.up=-q.up;
        return q;
    }

//  解析'/'后第二段数码。

    if(temp=='/'){_after_slash_:
        temp=fgetc(fp);
        switch(temp){
            case '-': pozneg=-pozneg;
            case '+': temp=fgetc(fp);
        }if(_IS_DIGIT_(temp))
            q.down=0;
        _ADD_DIGIT_(q.down,temp){
            // 这是为了误导IDE以便折叠代码块（这段全是溢出解析）。
            if(_IS_DIGIT_(temp)){ // 看看分子分母谁溢出更多。
                while(_IS_DIGIT_(temp))
                --count, temp=fgetc(fp);
            }ungetc(temp,fp);
            if(count){ // 分子分母溢出程度不同。
                if(count>=_D_L_O_R_)
                    return (rat){pozneg>0 ? RAT_MAX:RAT_MIN,1,1};
                if(count<=-_D_L_O_R_)
                    return rZERO;
                else{ // 快速幂。
                    ldb power=1,base;
                    if(count<0) count=-count,base=0.1L;
                    else base=10;
                    while(count){
                        if(count&1) power*=base;
                        base*=base; count>>=1;
                    }if(pozneg<0) q.up=-q.up;
                    return LdbToRat(q.up/(ldb)q.down*power);
                }
            }
        }if(pozneg<0) q.up=-q.up;
        q.splfd=0; return RatSimp(q);
    }

//  解析'.'后第二段数码。

    else{ _after_point_:;
        rat qq=rZERO;
        urint down=1;
        temp=fgetc(fp);
        _POINT_ADD_DIGIT_(qq.up,down,temp)
        qq.down=down;
        qq.splfd=0;
        q=RatPlus(q,qq);
        // 发生溢出，丢弃余下小数部分。
        if(_IS_DIGIT_(temp)){
            _SKIP_DECIMAL_PART_
            if(pozneg<0) q.up=-q.up;
            return q;
        }// 遇到左括号则放行到循环节。
        if(temp!='('){
            ungetc(temp,fp);
            if(pozneg<0) q.up=-q.up;
            return q;
        }// 以下是循环节模块。
        qq=(rat){0,down,0};
        temp=fgetc(fp);
        _POINT_ADD_DIGIT_(qq.up,qq.down,temp)
        // 防止括号间无内容。
        if(qq.down!=down){
            if(_IS_DIGIT_(temp)){
        // 解析循环节时发生溢出，剥夺它的循环节身份。
                do temp=fgetc(fp);
                while(_IS_DIGIT_(temp));
                q=RatPlus(q,qq);
            }else{
        // 注意到 1/9...9 可构造元循环节。
                qq.down-=down;
                q=RatPlus(q,qq);
            }if(temp!=')')
            ungetc(temp,fp);
        }else{
            ungetc(temp,fp);
            ungetc('(',fp);
        }if(pozneg<0) q.up=-q.up;
        return q; // q刚才在RatPlus里化简过了。
    }
}

    /*rat 综合输入（整数、分数、小数）*/
rat GetRat() {return fGetRat(stdin);}

#undef rint
#undef urint
#undef ldb
#undef _BETW_RTDT_
#undef _BETW_RTEP_
#undef _BETW_SQRTRM_
#undef _IS_SAFE_
#undef _PUTU_PPI_
#undef _PUT_CHECK_1_
#undef _PUT_CHECK_2_
#undef _DOB_SIZE_
#undef _IS_BLANK_
#undef _IS_DIGIT_
#undef _ADD_DIGIT_
#undef _POINT_ADD_DIGIT_
#undef _OVERFLOW_STATS_
#undef _SKIP_DECIMAL_PART_

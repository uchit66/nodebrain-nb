/*
* Copyright (C) 1998-2009 The Boeing Company
*                         Ed Trettevik <eat@nodebrain.org>
*
* NodeBrain is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place Suite 330, Boston, MA 02111-1307, USA.
*
*=============================================================================
* Program:  NodeBrain
*
* File:     nbpke.c
*
* Title:    Public Key Encryption Routines
*
* Function:
*
*   This file provides routines that implement NodeBrain public key
*   encryption. 
*
* Synopsis:
*
*   #include "nbpke.h"
*
*   unsigned int pkeCipher(ciphertext,exponent,modulus)
*        unsigned short *ciphertext,*exponent,*modulus;
*   unsigned int pkeEncrypt(ciphertext,exponent,modulus,plaintext)
*        unsigned short *ciphertext,*exponent,*modulus,*plaintext;
*   unsigned int pkeDecrypt(ciphertext,exponent,modulus,plaintext)
*        unsigned short *ciphertext,*exponent,*modulus,*plaintext;
*
* Description
*
*   pkeCipher(ciphertext,exponent,modulus)
*        - Implement encryption formula C[i]=P[i]^E mod N
*   
*   pkeEncrypt(ciphertext,exponent,modulus,plaintext)
*        - Encrypt plaintext to ciphertext.
*   
*   pkeDecrypt(ciphertext,exponent,modulus,plaintext)
*        - Decrypt ciphertext to plaintext.
*   
*   Each of these routines returns the length of the ciphertext buffer.
*
*=============================================================================
* Credits:
*
*   The pkeCipher routine is based on the RSA algorithm first published by
*   Ron Rivest, Adi Shamir, and Leonard Adleman in 1977.
*
*=============================================================================
* Change History: 
*
*    Date    Name/Change
* ---------- -----------------------------------------------------------------
* 2000-03-16 Ed Trettevik (original prototype version)
* 2000-03-28 eat
*             1) Included blocking and deblocking logic.
* 2000-05-27 eat - version 0.2
*             1) Removed dependence on vli word size.  This was enabled by
*                changes to the vli routines.
* 2000-10-09 eat - version 0.2.2
*             1) minor change to pkeGenKey for windows compiler
* 2001-02-08 eat - version 0.2.4
*             1) Modified to support binary data - previously only supported
*                text.  This required the addition of a "plaintext" length
*                parameter and a method of storing the length in the encrypted
*                text.
*             2) Restricted a ciphertext buffer to 256 bytes.  This restriction
*                is appropriate since bulk data encryption is performed with
*                secret key encryption.  This public key encryption is only
*                used for authentication strings and the exchange of randomly
*                generated secret keys.
* 2001-03-04 eat - version 0.2.4
*             1) Calls to vlimod have been changed to vlidiv to adjust to
*                changes in nbvli.h (this is just a name change).  A new
*                vlimod routine is used internal to vlipow for enhanced
*                performance.
*             2) Changed the way the bits for p are selected in pkeGenKey to
*                make it more random.
*             3) Increased temp vli variables from 512 bit to 2048 bit.
*
* 2008-11-11 eat 0.7.3  Change failure exit code to NB_EXITCODE_FAIL
*=============================================================================
*/
#include "nbi.h"
#include "nbvli.h"
#include "nbpke.h"

/*
*  The NodeBrain very large integer arithmetic routines included above
*  operate on integers in vli format.  We don't care about that format here
*  because we use vligetb and vliputb to get and put vli values from byte
*  arrays of the form below.
*
*      [least_significant_byte]...[most_significant_byte] 
*
*  We do, however, have vli variables which we define as "vli2048".  This
*  date type is sufficient to hold a 2048 bit integer, which will allow up to
*  1024 bit encryption.  The definition for vli2048 is provided in nbvli.h so
*  we don't need to understand it here.
*/

void pkePrint(ciphertext) unsigned char *ciphertext; {
  unsigned char *cursor=ciphertext;
  unsigned short len;
  
  len=*cursor;
  printf("ct[%u]=",len);
  for(;len>0;len--){
    printf("%2.2x",*cursor);
    cursor++;
    }
  printf(".\n");    
  } 
    
/*
*  Encipher or decipher a ciphertext buffer
*
*    [len][[data1][data2]...][...]...
*
*    The len field is an unsigned char (1-255) providing the length of the
*    ciphertext buffer (including len itself).  A 5 byte field is shown hear.
*
*        0x05,0x47,0xaf,0x2d,0x3e
*
*    Return value:
*        0 - modulus divided evenly into the ciphertext
*       >0 - modulus does not match the ciphertext block length
*    
*/  
unsigned int pkeCipher(ciphertext,exponent,modulus)
  unsigned char *ciphertext;
  vli exponent,modulus; {
  vli2048 T;
  unsigned char *cursor=ciphertext+1,*lastblock;
  unsigned int blocksize;	

  blocksize=vlibytes(modulus);              /* get block size from modulus */
  lastblock=ciphertext+*ciphertext-blocksize;
  if(lastblock<cursor) return(1);           /* not enough ciphertext */
  for(;cursor<=lastblock;cursor+=blocksize){
    vligetb(T,cursor,blocksize);
    vlipow(T,modulus,exponent);
    vliputb(T,cursor,blocksize);  
    }
  return(lastblock+blocksize-cursor);  
  }


/*
*  Encrypt a text message and return the total length of the ciphertext
*
*    For each N byte ciphertext block within the buffer, N-1 plaintext
*    bytes are stored.  The last byte is filled with a low value.
*
*            [byte1][byte2]...[byteN-1][0x00]
*
*    In the last block, the second to last byte provides the number of
*    data bytes.  As a last block, the following block only contains two
*    bytes of data (0x34,0x40).  As any other block, it would contain
*    five bytes of data.  We are assuming a 6 byte blocksize.  
*
*            0x34,0x40,0x21,0x78,0x02,0x00
*
*    After the plaintext is blocked as described above, pkeCipher is
*    called to encrypt the complete buffer (set of blocks).
*
*/
unsigned int pkeEncrypt(ciphertext,exponent,modulus,plaintext,length)
  vli exponent,modulus;
  unsigned char *ciphertext,*plaintext;
  unsigned int length; {
  
  unsigned char *in,*out,*end;
  unsigned int blocksize,len,inblocksize;

  blocksize=vlibytes(modulus);
  inblocksize=blocksize-1;
  end=plaintext+length-inblocksize;
  out=ciphertext+1;
  for(in=plaintext;in<=end;in+=inblocksize){   /* for each block */
    memcpy(out,in,inblocksize);
    out+=inblocksize;
    *out=0;
    out++;
    }
  len=end+inblocksize-in;
  memcpy(out,ciphertext+1,inblocksize-1);
  if(len>0) memcpy(out,in,len);
  out+=inblocksize-1;
  *out=len;
  out++;
  *out=0;
  out++;
  len=out-ciphertext;
  *ciphertext=len;
  pkeCipher(ciphertext,exponent,modulus);
  return(len);
  }  

/*
*  Decrypt a cipher text buffer into plaintext (binary) and return the plaintext length.
*
*/
unsigned int pkeDecrypt(ciphertext,exponent,modulus,plaintext,length)
  vli exponent,modulus;
  unsigned char *ciphertext,*plaintext;
  unsigned int length; {

  unsigned char *ct=ciphertext+1,*clastblock,*pt=plaintext,*pend;
  unsigned int cblocksize,pblocksize,part;

  if(pkeCipher(ciphertext,exponent,modulus)!=0) return(0); /* ciphertext & modulus mismatch */
  cblocksize=vlibytes(modulus);
  pblocksize=cblocksize-1;
  pend=plaintext+length-pblocksize;     
  clastblock=ciphertext+*ciphertext-cblocksize;
  for(;ct<clastblock;ct+=cblocksize){      /* for each block */
    if(pt>pend) return(0);                 /* avoid plaintext buffer overflow */
    memcpy(pt,ct,pblocksize);
    pt+=pblocksize;
    }  
  clastblock=ct+cblocksize-2;
  part=*clastblock;  
  if(part>(cblocksize-2)) return(0);       /* part out of range */
  if(pt>plaintext+length-part) return(0);  /* avoid plaintext buffer overflow */       
  memcpy(pt,ct,part);
  pt+=part;
  return(pt-plaintext);  
  }

/*
*  Test the encryption and decryption routines for a given key.
*/    
void pkeTestCipher(e,n,d) vli e,n,d; {
  unsigned char ciphertext[1024];
  unsigned int len,slen;
  unsigned char s[256],t[256];
    
  strcpy(s,"abcdefghijklmnopqrstuvwxyz0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ~!@#$%^&*()_+{}[]`,./<>?;':|\\\"");
  slen=strlen(s);
  slen=rand()%slen;
  *(s+slen)=0;
  strcpy(t,s); 
  len=pkeEncrypt(ciphertext,e,n,s,slen);
  len=pkeDecrypt(ciphertext,d,n,s,slen);
  if(len!=slen){
    printf("NB000E String encryption error - pkeDecrypt returned wrong length=%u.\n",len);
    printf("in : %s\n",t);
    exit(NB_EXITCODE_FAIL);
    }
  *(s+slen)=0;  
  if(strcmp(t,s)>0){
    printf("NB000E String encryption error on first try.\n");
    printf("in : %s\n",t);
    exit(NB_EXITCODE_FAIL);
    }
  pkeEncrypt(ciphertext,d,n,s,slen);
  pkeDecrypt(ciphertext,e,n,s,slen);
  *(s+slen)=0;
  if(strcmp(t,s)>0){
    printf("NB000E String encryption error on second try.\n");
    printf("in : %s\n",t);
    exit(NB_EXITCODE_FAIL);
    }
  }

/*********************************************************************
*  Routines to calculate encryption and decryption exponents using
*  the Extended Euclid's Algorithm.
*********************************************************************/  
void pkegetk();
/*
*  Get decryption key d for given encryption key e and modulus m.
*
*    e*d=mk+1  (e and m are known)
*
*    d=f*k+(r*k+1)/e
*   
*    x is e
*    y is m
*/ 
void pkegetj(j,x,y) vli j,x,y; {
  /* (xj-1)/y is an integer */
  /* j=(y/x)k+((y%x)k+1)/x */
  vli2048 f,k,r,p;

  vlicopy(r,y);
  vlidiv(r,x,f);  /* f=floor(y/x); r=y-f*x; */
  if(*r==0){
    printf("pkegetj Remainder is zero.  Something is wrong.\n");
    exit(NB_EXITCODE_FAIL);
    }
  if(*r==1 && *(r+1)==1) {
    vlicopy(k,x);
    vlidec(k);   /* k=x-1 */
    }
  else pkegetk(k,r,x);
  vlimul(f,k,j);
  vlimul(r,k,p);
  vlicopy(r,p);
  vliinc(r);
  vlidiv(r,x,f);
  vliadd(j,f);            /* j=f*k+(y*k+1)/x; */  
  }
  
void pkegetk(k,x,y) vli k,x,y; {
  /* (xk+1)/y is an integer */ 
  /* (xk+1)=yj */
  /* xk=yj-1 */
  /* k=(yj-1)/x */ 
  /* k=floor(y/x)*j+((y%x)j-1)/x */
  vli2048 f,j,r,p;

  vlicopy(r,y);
  vlidiv(r,x,f);  /* f=floor(y/x); r=y-f*x; */
  if(*r==0){
    printf("pkegetk Remainder is zero.  Something is wrong.\n");
    exit(NB_EXITCODE_FAIL);
    }
  if(*r==1 && *(r+1)==1){
    vlicopy(k,f);
    return;
    }
  else pkegetj(j,r,x);
  vlimul(f,j,k);
  vlimul(r,j,p);
  vlicopy(r,p);
  vlidec(r);
  vlidiv(r,x,f);
  vliadd(k,f);             /* k=f*j+(r*j-1)/x; */
  }

/*
*  Test encryption key on random vli numbers
*/  
void pkeTestKey(c,e,n,d) int c; vli e,n,d; {  
  vli2048 x,X;
  vliWord *cX,*cx,*ex;
  int i,l;

  l=vlibits(n)-1;           /* number of used bits in n */
  for(i=0;i<c;i++){         /* for c times */
    vlirand(x,l);           /* get random number */
    ex=x+*x+1;    
    vlicopy(X,x);           /* save random vli */
    vlipow(x,n,e);
    vlipow(x,n,d);
    cX=X;
    for(cx=x;cx<ex;cx++){
      if(*cx!=*cX){
        printf("NB000E Integer encryption error\n");
        vliprint(x,"x");
        vliprint(X,"X"); 
        exit(NB_EXITCODE_FAIL);
        }
      cX++;
      }
    }
  }
  
/*
* Generate key with an l bit modulus
*/
void pkeGenKey(l,e,n,d) unsigned int l; vli e,n,d; {    
  unsigned int b;
  vli2048 p,q,m;
  static long seed=0;

  if(l<9 || l>1024){
    printf("NB000L pkeGenKey: parameter l=%u is out of range.\n",l);
    exit(NB_EXITCODE_FAIL);
    }
  if(seed==0) srand(seed=time(NULL)); /* seed the random number generator */
  /* calculate p and q */
  b=rand()%l;
  if(b<2) b=2;
  vlirand(p,b);
  vlipprime(p);           /* increment p to a probable prime */
  vlirand(q,l-b);
  vlipprime(q);           /* increment q to a probable prime */
  /* calculate n and m */    
  vlimul(p,q,n);
  vlidec(p);
  vlidec(q);
  vlimul(p,q,m);          /* m=(p-1)*(q-1) */
  vliinc(p);
  vliinc(q);
  /* calculate e and d */
  vligeti(e,2); 
  vlirprime(e,m);         /* increment e to first value relatively prime to m */
  pkegetj(d,e,m);         /* get decryption key */
  pkeTestKey(10,e,n,d);   /* test on ten random numbers */
  }

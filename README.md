pebbleOTP
=========

pebbleOTP

Time based One Time Password generator for pebble


Supported OTP types
-------------------
* RFC6238 OTP (Google Authenticator)
* Battle.net OTP

Build Instruction
-----------------
1. Get your OTP Secrets
  * Secrets for Google Authenticator is already base32 encoded. just use it as
  * Battle.net OTP Secret can be obtained by using python-bna(https://github.com/Adys/python-bna)
    * python-bna saves secret in "~/.config/bna/bna.conf". this secret is hexlified. you should unhexlify and base32 encode it.
      ```python
      import base64, binascii
      print base64.b32encode(binascii.unhexlify('YOUR_BATTLE_NET_SECRET'))
      ```
2. edit `src/main.c` to input the your OTP info
  * edit `OTP_INFO`.
    * `type` can be `totpGoogle` or `totpBattlenet`
    * `secret` is your base32 encoded secret.
    * `description` is description.
    * You want to add more OTP, just expand OTP_INFO variable. here is an example.
      ```c
      OTPInfo OTP_INFO[] = {
        {
          .type = totpGoogle,
          .secret = "secretsecret",
          .description = "Github"
        }, 
        {
          .type = totpBattlenet,
          .secret = "battlenetotpsecret",
          .description = "Battle.net OTP!"
        }, 
      };
      ```
3. edit `src/totp.c` to setup default timezone
  * variable `TIMEZONE` is an index for `TIMEZONE_MAP`.
    find your timezone offset from UTC in `TIMEZONE_MAP` and set `TIMEZONE` to your timezone index.
    
4. Just build it and install on your pebble.
  

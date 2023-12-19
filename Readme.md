# pam-riddle
A PAM module that takes riddles from `/etc/riddles` and checks the answer against the value in there

## Configuration file
the `riddles` file is separated by | and is in the format `question|answer`
Each entry is separated by a newline character `\n`

```
What gets wetter the more it dries?|towel
What has branches but no fruit or leaves?|bank
```

## Usage
In order to use PAM-Riddle you need to include it in a PAM configuration within `/etc/pam.d/` for example `/etc/pam.d/sudo` for sudo access

Example config:
```
#%PAM-1.0

# Set up user limits from /etc/security/limits.conf.
session    required   pam_limits.so
auth       required   pam_riddle.so

@include common-auth
@include common-account
@include common-session-noninteractive
```

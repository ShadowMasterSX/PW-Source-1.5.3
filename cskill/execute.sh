cd skill;
find . -name "skill*.h" -type f -exec sh -c 'iconv -f GBK -t UTF-8 "{}" > "{}.utf8" && mv "{}.utf8" "{}"' \;
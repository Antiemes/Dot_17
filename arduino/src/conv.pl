#!/usr/bin/perl -w

open(IN, "demo.txt") || die("File cannot be opened.\n");
print "static const uint8_t PROGMEM scroll[3][128]={\n";
for ($row=0; $row<0; $row++)
{
  <IN>;
}
for ($row=0; $row<=2; $row++)
{
  print "{";
  chomp($line=<IN>);
  for ($x=0; $x<length($line); $x+=8)
  {
    $s=substr($line, $x, 8);
    $s=reverse($s);
    $s=~s/[ .]/0/g;
    $s=~s/[X#]/1/g;
    $n=eval("0b".$s);
    print $n;
    print ", ";
  }
  print "},";
  print "\n";
}
close(IN);
print "};\n";

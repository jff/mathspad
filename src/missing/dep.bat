nm -g libname.so > file.libname
grep 'GLOB' file.libname | \
awk -F'|' '{ print $8" "$7" "$1; }' | \
sed 's#file.lib\([^:]*\):.*$#\1#g' | \
sort | \
awk ' BEGIN \
	{ name=""; libname=""; } \
      ($2 == "UNDEF") && ($1 == name) \
	{ print $3" -> " libname " " name ; \
	  next; } \
      $2 == "UNDEF" \
	{ print $3" -> system "$1; \
	  next; } \
        { name=$1; \
	  libname=$3; \
	  next; } ' | \
sort | \
awk ' BEGIN \
	{ libname=""; depname="";} \
      $1\!= libname \
	{ print ""; \
	  print "@"; \
	  print "Dependencies for "$1; \
	  libname=$1; \
	  depname=""; } \
      $3\!=depname \
	{ depname=$3; \
	  print ""; \
	  print ""; \
	  print depname" library"; \
	  printf "^";} \
	{ printf $4", "; }' | \
tr '@^' '\014\011'

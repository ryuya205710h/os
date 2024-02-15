#!/usr/bin/perl

use strict;

my $width = 1200;
my $step = $width;
my $m_height = 3;
my $base = 0;


# my $malloc_test = "./a.out -m 10000000 -l 10 -u 16";
# my $malloc_test = "ssh urasoe.ie.u-ryukyu.ac.jp ./a.out -m 10000000 -l 10 -u 16";
# my $malloc_test = "ssh urasoe.ie.u-ryukyu.ac.jp ./a.out -u 16 -m 1000000 -l 10";
my $malloc_test = "ssh amane.ie.u-ryukyu.ac.jp ./malloc_test -m 10000000 -l 10";
# my $malloc_test = "./malloc_test -m 100000 -l 10";
# my $malloc_test = "./malloc_test -m 10000000 -l 10";

open(TEST,"$malloc_test | sort|") or die("Cannot run $malloc_test $!\n");
my ($min,$max);
my @adr;
my @size;

while(<TEST>) {
    chop;
    my ($adr,$size) = split;
    $adr = eval($adr);
    $size = eval($size);
    $min = $adr if ($min==0||$adr<$min);
    $max = $adr if ($adr>$max);
    push(@adr,$adr);
    push(@size,$size);
}

# print "$width $max $min\n";
my $scale = $step/($max-$min);

# print "$scale $width $max $min\n";
# $m->update;
# exit;

my @data;

for(my $i =0; $i<$#adr;$i++) {
    my $adr = ($adr[$i]-$min)*$scale;
    my $size = $size[$i]*$scale;

    my $y = int($adr/$width) * $m_height + $base;
    my $x = int($adr)%$width;

# print STDERR "$adr $size $x $y\n";
    push(@data,  $x,$size );
}

my $xydata = "[" . join(",",@data) ."]";
my $count = int ($#data / 2);

my $js = << "EOFEOF";
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>D3 World Map</title>
    <script src="https://d3js.org/d3.v4.min.js"></script>
    <style>
        #halfpage{
            background: "white";
            overflow-x: scroll;
        }
    </style>
  </head>
  <body>
  <button type="button" onclick="shrink()" >shrink </button>
  <button type="button" onclick="enlarge()" >enlarge</button>

  <script type="text/javascript">

  var elements = Array.from(Array($count).keys());
  var xydata = $xydata;
  var scale = 1;

  var shrink = function(){
      scale = scale * 0.7 ;
      svgContainer.remove();
      svgUpdate(scale,"green");
  }

  var enlarge = function(){
      scale = scale / 0.7 ;
      svgContainer.remove();
      svgUpdate(scale,"green");
  }

  var svgUpdate = function(scale,color){
     svgContainer = d3.select("body").append("svg")
    .attr("width", 1200 * scale)
    .attr("height", 300)
    .attr("id", "halfpage");

    svgContainer.selectAll("rects1").data(elements).enter().append("rect")
                .attr("width", function(d,i) { return xydata[i*2+1] * scale;} )
                .attr("height", 20)
                .attr("y", 175)
                .attr("x", function(d,i) { return xydata[i*2] * scale; })
                .style("fill", color)
                .style("stroke-width", 1)
       	        .style("stroke", "black");
  }

   svgUpdate(scale,"green");

    </script>
  </body>
</html>
EOFEOF

print "$js";


# end

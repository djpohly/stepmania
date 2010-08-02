#!/usr/bin/perl -w

use strict;
use File::Copy;
use File::Path;
use File::Basename;
use File::Temp qw/tempfile tempdir/;
use Cwd;

my @docs = ( "Licenses.txt" );

# Passing a date for a CVS release gives StepMania-CVS-date.
# Otherwise you get StepMania-ver.
die "usage: $0 [date]\n" if @ARGV > 1;
my $root = getcwd;
my $scripts = dirname $0;
my $srcdir = ( $scripts =~ m{^/} ? "$scripts/../.." :
	       "$root/$scripts/../.." );
my $family;
my $id;
my $ver;

open FH, "$srcdir/src/ProductInfo.h" or die "Where am I?\n";
while( <FH> )
{
	if( /^#define\s+PRODUCT_FAMILY_BARE\s+(.*?)\s*$/ ) {
		$family = $1;
	} elsif( /^#define\s+PRODUCT_ID_BARE\s+(.*?)\s*$/ ) {
		$id = $1;
	} elsif( /^#define\s+PRODUCT_VER_BARE\s+(.*?)\s*$/ ) {
		$ver = $1;
	}
	
}
close FH;

my $destname = @ARGV ? "$id-$ARGV[0]" : "$family-$ver";

$destname =~ s/\s+/-/g;

my $tmp = tempdir;
my $dstdir = "$tmp/$id";
mkdir $dstdir;

# Copy StepMania
system 'cp', '-r', "$srcdir/StepMania.app", $dstdir and die "cp -r failed: $!\n";
system 'strip', '-x', "$dstdir/StepMania.app/Contents/MacOS/StepMania";

# SUPER HACK: Setting the position of the first non-hidden file icon doesn't take effect for some reason.  Figure out why eventually so that we can remove 0dummy.
system 'cp', 'scripts/0dummy', "$tmp/" and die "ln -s failed: $!\n";
system 'cp', 'scripts/Applications', "$tmp/" and die "ln -s failed: $!\n";

#system "$srcdir/Utils/CreatePackage.pl", $srcdir, "$dstdir/StepMania.app/Contents/Resources" and die "mksmdata.pl failed: $!\n";

my @files = (
	"Announcers",
	"BackgroundEffects",
	"BackgroundTransitions",
	"BGAnimations",
	"CDTitles",
	"Characters",
	"Courses/Samples",
	"Data",
	"NoteSkins/common/default",
	"NoteSkins/dance/default",
	"NoteSkins/dance/flat",
	"Packages",
	"RandomMovies",
	"Songs",
	"Themes/default",
);
foreach my $x ( @files )
{
	print "$dstdir/" . dirname $x . "\n";
	mkpath "$dstdir/" . dirname $x;
	system 'cp', '-r', "$srcdir/$x", "$dstdir/$x" and die "cp -r failed: $!\n";
}


# clean up .svn dirs
my @svndirs = split /\n/, `find "$dstdir" -type d -name .svn`;
rmtree \@svndirs;


# Make a dmg (old)
#system qw/hdiutil create -ov -format UDZO -imagekey zlib-level=9 -srcfolder/, $tmp, '-volname', $destname, "$root/$destname-mac.dmg";
my $dmg = "$root/$destname-mac.dmg";

# Make a dmg (new)
system 'rm', $dmg;
system 'sh', 'scripts/yoursway-create-dmg-1.0.0.2/create-dmg', '--window-size', '750', '370', '--background', 'scripts/background.png', '--icon-size', '64', '--volname', "$family $ver", '--icon', '0dummy', '600', '250', '--icon', 'Applications', '450', '250', '--icon', $id, '270', '250', $dmg, $tmp;


rmtree $tmp;


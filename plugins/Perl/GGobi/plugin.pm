use GGobi;

{
=head GGobi::GGobiGenericRef
 This is the lowest-level base class for 
 representing a reference to a C-level data structure
 instance in Perl. We extend this class to provide 
 interfaces to different types.
=cut
    package GGobi::GGobiGenericRef;

    sub new {
	my ($class, $ptr) = @_;
        my $this = $ptr; # { 'address' => $ptr };

	my $x = bless \$this, $class;
	return($x);
    }
}

{
=head GGobi::GGobiRef
  This is a proxy/reference in Perl for a C-level
  ggobid instance.
  
=cut
    package GGobi::GGobiRef;
    our @ISA = qw(GGobi::GGobiGenericRef);

    sub numDatasets {
        my $this = shift;
	@x = GGobi.dataSetNames($this);
        return($#x + 1);
    }

    sub DESTROY {
	print "GGobiRef::Destroy\n";
    }
}

{
    package GGobi::PluginInstRef;
    our @ISA = qw(GGobi::GGobiGenericRef);
sub DESTROY {
    print "PluginInstRef::Destroy\n";
}
}


{
=head GGobi::Plugin
 This is the standard base class for a Plugin
 that provides the basic methods that are called
 directly from the GGobi-Perl plugin mechanism.
 One can extend this and be certain that all the
 relevant methods are provided and in a degenerate
 fashion so as to cause no extra computation.
=cut
 package GGobi::Plugin;

 sub new() {
    print "[ggobi::plugin] In onCreate\n";
    my $class = shift;
    my $this = 1;
    bless \$this, $class;
 }


=head onClose
  This method is currently never invoked.
  The DESTROY method is where we clean up when 
  plugin instance is no longer needed.
=cut
 sub onClose() {
    print "[ggobi::plugin] In onClose\n"; 
    return(1);
 }

=head updateDisplayMenu
 This is the method that is called when a new dataset is added to 
 the GGobi instance and so the Display menu in the toolbar is augmented
 with controls for the new dataset. 
 If this plugin needs to add to this menu for a dataset,
 it can do this at this point.
=cut
  sub updateDisplayMenu {
      my $this = shift;
      print "[updateDisplayMenu] ", ref($this), "\n";

      return(1);
  }
}

# print "Loaded plugin.pm\n";

=comment


# These two Arg* classes are not used.
{
    package GGobi::ArgList;
sub new {
    my $class = shift;
    my @this = (@_);

    bless \@this, $class;
}
}

{
    package GGobi::ArgTable;
    sub new {
	my $class = shift;
	my %this = @_;

	bless \%this, $class;
    }
}

=cut 

1;



# ****************************************************************************
#
# Intense.pm : Intense Perl module
#
# Copyright 2001, 2002 Paul Swoboda.
#
# This file is part of the Intense project.
#
# Intense is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Intense is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Intense; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
#
# ****************************************************************************

package Intense;

use 5.008;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use Intense ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(
	
) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(
cswitch cget cset crefset cmod
);

our $VERSION = '0.01';

require XSLoader;
XSLoader::load('Intense', $VERSION);


# Preloaded methods go here.


my $C = new Context();


sub cset
{
  if ($#_ >= 0) {
    $C = new Context(shift);
  } else {
    $C = new Context();
  }
}


sub cget
{
  return $C;
}


sub crefset
{
  if ($#_ == 0) {
    my $newC = shift;
    if ((!ref $newC)||(!UNIVERSAL::isa($newC, "Context"))) {
      die "non-Context argument given to Intense::crefset()."
    } else {
      $C = $newC;
    }
  } else {
    die "Intense::crefset() takes one one source Context argument."
  }
}


sub cmod
{
  if ($#_ == 0) {
    my $op = shift;
    if (!ref $op) {
      # We assume it's a parsable op:
      my $contextOp = new ContextOp();
      if ($contextOp->parse($op)) {
        die "non-parsable ContextOp string given to Intense::cmod().";
      }
      $C->apply($contextOp);
    } elsif (!UNIVERSAL::isa($op, "ContextOp")) {
      die "non-ContextOp object argument given to Intense::cmod()."
    } else {
      $C->apply($op);
    }
  } else {
    die "Intense::cmod() takes one one source ContextOp argument."
  }
  return $C;
}


sub cswitch
{
  my $requested;
  my $protoRequested = undef;
  my $args = undef;
  my $blocks;

  if ($#_ < 0) {
    die "Need at least one arg to cswitch.";
  } elsif ($#_ == 0) {
    # cswitch [blocks]
    $requested = $C;
  } elsif ($#_ == 1) {
    if ((ref $_[0]) eq 'ARRAY') {
      # cswitch [args] [blocks]
      $requested = $C;
      $args = shift;
    } elsif ((ref $_[0])&&
         UNIVERSAL::isa($_[0], 'Context')) {
      # cswitch context [blocks]
      $requested = shift;
    } else {
      # cswitch proto-context [blocks]
      $protoRequested = shift;
    }
  } elsif ($#_ == 2) {
    if ((ref $_[0])&&UNIVERSAL::isa($_[0], 'Context')) {
      # cswitch context [args] [blocks]
      $requested = shift;
    } else {
      # cswitch proto-context [args] [blocks]
      $protoRequested = shift;
    }
    if (ref($_[0]) ne 'ARRAY') {
      die "Non-arrayref (type=" . ref $_[0] . ") given as argument " .
          "list to Intense::cswitch.";
    }
    $args = shift;
  } else {
    die "More than three arguments given in Intense::cswitch.";
  }
  if (ref($_[0]) ne 'ARRAY') {
    die "Non-arrayref (type=" . ref $_[0] . ") given as block list " .
        "argument to Intense::cswitch.";
  }
  $blocks = shift;
  if (defined $protoRequested) {
    $requested = new Context();
    if ($requested->parse($protoRequested)) {
      die "Non-parsable requested version argument given to Intense::cswitch.";
    }
  }
  # Now we can build the context domain:
  my $domain = new ContextDomain;
  my @contexts;
  foreach my $arg (@{$blocks}) {
    if (!ref $arg) {
      # We assume it's a parsable context:
      my $context = new Context;
      if ($context->parse($arg)) {
        die "non-parsable Context string given in " .
            "Intense::cswitch block list.";
      }
      push @contexts, $context;
    } elsif (UNIVERSAL::isa($arg, "Context")) {
      push @contexts, $arg;
    } elsif ((ref $arg) eq 'CODE') {
      $domain->insert($arg, @contexts);
      @contexts = ();
    } else {
      die "Reference given as block list element to Intense::cswitch was " .
          "neither a Context reference nor a code reference.";
    }
  }
  my $best = $domain->best($requested);
  if (!defined $best) {
    return undef;
  }
  if (defined $args) {
    return &{$best}(@{$args});
  } else {
    return &{$best};
  }
}


package Context;


use overload (
  '""' => \&stringify,
  '+=' => \&apply,
  '+' => \&applyToTemp
);


sub new
{
  my $self;
  my $classname = shift;

  if ($#_ >= 0) {
    my $arg = shift;
    if (ref $arg) {
      $self = _copyConstructor($arg);
    } else {
      $self = _parseConstructor($arg);
    }
  } else {
    $self = _voidConstructor();
  }
  return $self;
}


sub stringify
{
  my $self = shift;

  return $self->canonical();
}


sub apply
{
  my $self = shift;
  my $result = undef;

  if ($#_ >= 0) {
    do {
      my $arg = shift;
      if ((ref $arg)&&(UNIVERSAL::isa($arg, "Context"))) {
        $result = $self->_assignContext($arg);
      } elsif ((ref $arg)&&(UNIVERSAL::isa($arg, "ContextOp"))) {
        $result = $self->_applyContextOp($arg);
      } else {
        # Try to make a ContextOp from it:
        my $contextOp = new ContextOp();
        if (!defined $arg) {
          die "Undefined value passed as requested context to " .
              "Intense::Context::apply().";
        }
        if ($contextOp->parse($arg)) {
          die "non-parsable ContextOp string given to " .
              "Intense::Context::apply().";
        }
        $result = $self->_applyContextOp($contextOp);
      }
    } while ((!defined $result)&&($#_ >= 0));
  } else {
    die "No argument supplied to intense::Context::apply()";
  }
  return $self;
}


sub assign
{
  my $self = shift;
  my $result = undef;

  if ($#_ >= 0) {
    do {
      my $arg = shift;
      if ((ref $arg)&&(UNIVERSAL::isa($arg, "Context"))) {
        $result = $self->_assignContext($arg);
      } elsif ((ref $arg)&&(UNIVERSAL::isa($arg, "ContextOp"))) {
        die "Cannot assign a ContextOp to a Context";
      } else {
        # Try to make a Context from it:
        my $context = new Context();
        if (!defined $arg) {
          die "Undefined value passed as requested context to " .
              "Intense::Context::assign().";
        }
        if ($context->parse($arg)) {
          die "non-parsable Context string given to " .
              "Intense::Context::assign().";
        }
        $result = $self->_assignContext($context);
      }
    } while ((!defined $result)&&($#_ >= 0));
  } else {
    die "No argument supplied to intense::Context::assign()";
  }
  return $self;
}


sub applyToTemp
{
  my ($self, $o2) = @_;
  my $temp = new Context($self);

  return $temp->apply($o2);
}


sub can
{
  my $self = shift;

  return $self->canonical();
}


package ContextOp;


use overload (
  '""' => \&stringify,
  '+=' => \&applyOp,
  '+' => \&applyOpToTemp
);


sub new
{
  my $self;
  my $classname = shift;

  if ($#_ >= 0) {
    my $arg = shift;
    if (ref $arg) {
      $self = _copyConstructor($arg);
    } else {
      $self = _parseConstructor($arg);
    }
  } else {
    $self = _voidConstructor();
  }
  return $self;
}


sub stringify
{
  my $self = shift;

  return $self->canonical();
}


sub apply
{
  my $self = shift;
  my $result = undef;

  if ($#_ >= 0) {
    do {
      my $arg = shift;
      if ((ref $arg)&&(UNIVERSAL::isa($arg, "Context"))) {
        $result = $self->_applyContext($arg);
      } elsif ((ref $arg)&&(UNIVERSAL::isa($arg, "ContextOp"))) {
        $result = $self->_applyContextOp($arg);
      } else {
        # Try to make a ContextOp from it:
        my $contextOp = new ContextOp();
        if (!defined $arg) {
          die "Undefined value passed as requested context to " .
              "Intense::ContextOp::apply().";
        }
        if ($contextOp->parse($arg)) {
          die "non-parsable ContextOp string given to " .
              "Intense::ContextOp::apply().";
        }
        $result = $self->_applyContextOp($contextOp);
      }
    } while ((!defined $result)&&($#_ >= 0));
  } else {
    die "No argument supplied to intense::Context::apply()";
  }
  return $self;
}


sub assign
{
  my $self = shift;
  my $result = undef;

  if ($#_ >= 0) {
    do {
      my $arg = shift;
      if ((ref $arg)&&(UNIVERSAL::isa($arg, "Context"))) {
        $result = $self->_applyContext($arg);
      } elsif ((ref $arg)&&(UNIVERSAL::isa($arg, "ContextOp"))) {
        $result = $self->_assignContextOp($arg);
      } else {
        # Try to make a ContextOp from it:
        my $contextOp = new ContextOp();
        if (!defined $arg) {
          die "Undefined value passed as requested context to " .
              "Intense::ContextOp::assign().";
        }
        if ($contextOp->parse($arg)) {
          die "non-parsable ContextOp string given to " .
              "Intense::ContextOp::assign().";
        }
        $result = $self->_assignContext($contextOp);
      }
    } while ((!defined $result)&&($#_ >= 0));
  } else {
    die "No argument supplied to intense::ContextOp::assign()";
  }
  return $self;
}


sub applyOpToTemp
{
  my ($self, $o2) = @_;
  my $temp = new ContextOp($self);

  return $temp->apply($o2);
}


sub can
{
  my $self = shift;

  return $self->canonical();
}


package ContextBinder;


use overload (
  '""' => \&stringify
);


sub new
{
  my $self;
  my $classname = shift;

  if ($#_ < 0) {
    die "No bound-value argument given to Intense::ContextBinder constructor.";
  } else {
    my $bound = shift;
    $self = _boundConstructor($bound);
  }
  if ($#_ >= 0) {
    my $context = shift;
    if ((!ref $context)||(!UNIVERSAL::isa($context, "Context"))) {
      # Try to parse it:
      if ($self->parse($context)) {
        die "non-parsable Context string given to " .
            "Intense::ContextBinder constructor.";
      }
    } else {
      $self->assign($context);
    }
  }
  return $self;
}


sub stringify
{
  my $self = shift;

  return $self->canonical();
}


sub can
{
  my $self = shift;

  return $self->canonical();
}


sub apply
{
  my $self = shift;
  my $result = undef;

  if ($#_ >= 0) {
    do {
      my $arg = shift;
      if ((ref $arg)&&(UNIVERSAL::isa($arg, "Context"))) {
        $result = $self->_assignContext($arg);
      } elsif ((ref $arg)&&(UNIVERSAL::isa($arg, "ContextOp"))) {
        $result = $self->_applyContextOp($arg);
      } else {
        # Try to make a ContextOp from it:
        my $contextOp = new ContextOp();
        if (!defined $arg) {
          die "Undefined value passed as requested context to " .
              "Intense::ContextBinder::apply().";
        }
        if ($contextOp->parse($arg)) {
          die "non-parsable ContextOp string given to " .
              "Intense::ContextBinder::apply().";
        }
        $result = $self->_applyContextOp($contextOp);
      }
    } while ((!defined $result)&&($#_ >= 0));
  } else {
    die "No argument supplied to intense::ContextBinder::apply()";
  }
  return $self;
}


sub assign
{
  my $self = shift;
  my $result = undef;

  if ($#_ >= 0) {
    do {
      my $arg = shift;
      if ((ref $arg)&&(UNIVERSAL::isa($arg, "Context"))) {
        $result = $self->_assignContext($arg);
      } elsif ((ref $arg)&&(UNIVERSAL::isa($arg, "ContextOp"))) {
        die "Cannot assign a ContextOp to a ContextBinder";
      } else {
        # Try to make a Context from it:
        my $context = new Context();
        if (!defined $arg) {
          die "Undefined value passed as requested context to " .
              "Intense::ContextBinder::assign().";
        }
        if ($context->parse($arg)) {
          die "non-parsable Context string given to " .
              "Intense::ContextBinder::assign().";
        }
        $result = $self->_assignContext($context);
      }
    } while ((!defined $result)&&($#_ >= 0));
  } else {
    die "No argument supplied to intense::ContextBinder::assign()";
  }
  return $self;
}


package ContextDomain;


use overload (
  '&{}' => \&best
);


sub best
{
  my $self = shift;
  my $bestFitBinder = undef;
  if ($#_ >= 0) {
    do {
      my $arg = shift;
      if ((ref $arg)&&(UNIVERSAL::isa($arg, "Context"))) {
        $bestFitBinder = $self->bestBinder($arg);
      } else {
        # Try to make a context from it:
        my $context = new Context();
        if (!defined $arg) {
          die "Undefined value given as requested context to " .
	      "Intense::ContextDomain::best().";
        }
        if ($context->parse($arg)) {
          die "non-parsable Context string given to " .
	      "Intense::ContextDomain::best().";
        }
        $bestFitBinder = $self->bestBinder($context);
      }
    } while ((!defined $bestFitBinder)&&($#_ >= 0));
  } else {
    $bestFitBinder = $self->bestBinder($C);
  }
  if (defined $bestFitBinder) {
    return $bestFitBinder->bound();
  } else {
    return undef;
  }
}


sub near
{
  my $self = shift;
  if ($#_ >= 0) {
    my $arg = shift;
    if ((ref $arg)&&(UNIVERSAL::isa($arg, "Context"))) {
      return $self->nearBinders($arg);
    } else {
      # Try to make a context from it:
      my $context = new Context();
      if (!defined $arg) {
        die "Undefined value passed as requested context to " .
	    "Intense::ContextDomain::near().";
      }
      if ($context->parse($arg)) {
        die "non-parsable Context string given to " .
	    "Intense::ContextDomain::near().";
      }
      return $self->nearBinders($context);
    }
  } else {
    return $self->nearBinders($C);
  }
  return;
}


sub insert
{
  my $self = shift;

  if ($#_ < 0) {
    die "No argument given to Intense::ContextDomain::insert().";
  } elsif ($#_ == 0) {
    my $arg = shift;
    if ((!ref $arg)||(!UNIVERSAL::isa($arg, "ContextBinder"))) {
      die "Non-ContextBinder argument given to " .
	  "Intense::ContextDomain::insert().";
    }
    $self->insertBinder($arg);
  } else {
    my $bound = shift;
    my $binder;
    do {
      my $arg = shift;
      if ((!ref $arg)||(!UNIVERSAL::isa($arg, "Context"))) {
        $binder = new ContextBinder($bound, $arg);
      } else {
        # Try to make a context from it:
        my $context = new Context();
        if ($context->parse($arg)) {
          die "non-parsable Context string given to " .
	      "Intense::ContextDomain::insert().";
        }
        $binder = new ContextBinder($bound, $context);
      }
      $self->insertBinder($binder);
    } while ($#_ >= 0);
  }
}


package Intense;


1;


__END__
# Below is stub documentation for your module. You'd better edit it!

=head1 NAME

Intense - Perl extension for the Intense C++ intensional context library.

=head1 SYNOPSIS

  use Intense;

=head1 ABSTRACT

Intense (named for Intensional Programming) is a collection of C++
classes that allows the programmer to define tree-structured intensional
contexts of arbitrary arity, as well as corresponding, tree-structured
context operators.  This Perl module currently provides interfaces to the
Context, ContextOp, ContextBinder and ContextDomain classes.

=head1 DESCRIPTION

=head2 CONTEXTS

The basic class of the Intense collection is Context, a self-aggregating
tree node class with an arbitrary number of sub-context trees, mapped
under string "dimensions".  A reference to a Context object is a
reference to a point in a context tree; the tree is deallocated only when
the last reference to any point in the tree is invalidated by Perl.  Each
node of a context may have an optional "base value", currently accessible
in Perl as a string - the base values of a context are its content.

A context has a string representation, wherein every node of the tree is
enclosed in angle brackets, with the node's representation consisting of
an optional string (quoted) base value, followed by a (possibly empty)
list of sub-contexts terms, separated by '+', and each labelled with its
dimension under the current node, followed by a colon.  An example is:

    <"value one" + dim1:<dim2:<"value2"> + dim3:value3>>

Note that, for purposes of context parsing, base values (such as
"value3", above) do not have to be quoted if they contain only word
characters.  Sub-contexts can be indexed by "compound dimensions", where
the components of the dimension (each consisting only of alphanumeric
characters) are joined with colons.  The ordering of the base value and
sub-context terms is irrelevant, and dimensions are distributive over
sub-contexts.  For example, the above string representation is equivalent
to:

    <dim1:dim2:value2 + "value one" + dim1:dim3:value3>

In general, repetition of individual dimensions (i.e., context nodes) is
OK, with the caveat that repeated base values in the same context string
simply override the previous base value given for the same node in the
same string.

=head2 CONTEXT OPERATIONS

Context operations are idempotent and associative, regardless of depth or
complexity.  The application of a (right) operation to a (left) operation
yields a result operation that has the same effect, in application to a
context, as applying the original two operations to the context in
succession.  This holds for a sequence of operations of arbitrary length.

I.e., the following expressions are all equivalent:

    $context + $op1 + $op2 + $op3
    $context + ($op1 + $op2) + $op3
    $context + $op1 + ($op2 + $op3)
    $context + ($op1 + $op2 + $op3)
    $context + ($op1 + $op2 + $op3) + ($op1 + $op2 + $op3)
    ($context + $op1) + $op2 + $op3
    etc

=head2 CSWITCH ROUTINE

Modelled loosely on the "vswitch" expression from the language ISE,
cswitch allows the simplified runtime construction and best-fit calling
of a ContextDomain of blocks of code.  The block list is given as an
arrayref, containing one or more sub-lists of context expressions, each
sub-list followed by a single code reference.  The idea is that, when the
cswitch call is made, the ContextDomain is populated by registering each
given block (coderef scalar) under each of the context expressions (as
evaluated during the cswitch call) that precedes it.  That is, a
ContextBinder is constructed for each applicable version of each given
block and is inserted into the ContextDomain.  Following this, a best
fit is made on the registered blocks, using the requested context; if
the result is defined, the block is executed, optionally with any given
argument list.  The result of the cswitch is either undefined, or the
value returned from the executed block.

The form is:

cswitch [context expression] [argument list] <block list>

An example is as follows:

    cswitch ($context [$arg1, $arg2], [
        '<green>', makeContext($arg), $context3, sub {
            ...
        },
        '<>', '<red>', sub {
            ...
        },
        $c6, sub {
            ...
        }
    ]);

Currently, cswitch is implemented in Perl, although this could
easily be migrated to C++ in the future, if runtime efficiency is
found to be more important than code flexibility.

=head2 EXPORT

The routines cswitch, cset, cget, crefset, and cmod are exported.

=head1 AUTHOR

Paul Swoboda, E<lt>gnuDIEatSPAMMERShackomaticDIEdot!!net>

=head1 COPYRIGHT AND LICENSE

Copyright 2003 by Paul Swoboda

This library is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License
(http://www.fsf.org/licenses/gpl.html).

=cut

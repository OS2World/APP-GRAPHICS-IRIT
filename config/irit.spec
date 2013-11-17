Summary: A solid modeling environment
Name: irit
Version: 0.0.1
Release: 1
Source0: %{name}-%{version}.tar.gz
License: SPECIAL (Free for non commercial use)
Group: Development/Tools
%description
IRIT (www.cs.technion.il/~irit) is  a solid modeling environment  that
allows one  to model  basic, primitive   based, models  using  Boolean
operations as   well as freeform   surface's based models.  Beyond its
strong support  for Bezier and  Bspline curves and (trimmed) surfaces,
IRIT has several unique features  such as strong symbolic computation,
support  of trivariate spline  volumes,  multivariate spline functions
and triangular patches.  Numerous unique  applications such as surface
layout   decomposition, metamorphosis  of   curves  and surfaces,  and
artistic line art drawings of parameteric and  implicit forms.  A rich
set of computational geometry tools for freeform curves and surface is
offered, such as offsets, bisectors, convex hulls, diameters, kernels,
and distance  measures. The  solid  modeler is highly  portable across
different hardware   platforms, including  a   whole variety  of  Unix
machines and Windows PC.
See also GuIrit (www.cs.technion.il/~gershon/GuIrit), a graphical user
inetrface for Irit.
%prep
%setup -q
%build
./configure
make
%install
make install
%files
%defattr(-,root,root)
%doc COPYING AUTHORS README NEWS

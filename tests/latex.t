. tests/functions.sh

# don't run these tests unless --with-latex
./markdown -V | grep LATEX >/dev/null || exit 0

title "embedded latex"

rc=0
MARKDOWN_FLAGS=

try 'latex passthrough' '\(\tex\)' '<p>\(\tex\)</p>'
try 'latex w/ special characters' 'Equation:\(a<+>b\).' \
    '<p>Equation:\(a&lt;+&gt;b\).</p>'

try 'latex with $$ .. $$' '$$foo$$' '<p>$$foo$$</p>'
try 'latex with $$ .. $$ multi lines' '$$\begin{split}\nabla \times \vec{\mathbf{B}} -\, \frac1c\, \frac{\partial\vec{\mathbf{E}}}{\partial t} & = \frac{4\pi}{c}\vec{\mathbf{j}} \\   \nabla \cdot \vec{\mathbf{E}} & = 4 \pi \rho \\ \nabla \times \vec{\mathbf{E}}\, +\, \frac1c\, \frac{\partial\vec{\mathbf{B}}}{\partial t} & = \vec{\mathbf{0}} \\ \nabla \cdot \vec{\mathbf{B}} & = 0 \end{split}$$' '<p>$$\begin{split}\nabla \times \vec{\mathbf{B}} -\, \frac1c\, \frac{\partial\vec{\mathbf{E}}}{\partial t} &amp; = \frac{4\pi}{c}\vec{\mathbf{j}} \\   \nabla \cdot \vec{\mathbf{E}} &amp; = 4 \pi \rho \\ \nabla \times \vec{\mathbf{E}}\, +\, \frac1c\, \frac{\partial\vec{\mathbf{B}}}{\partial t} &amp; = \vec{\mathbf{0}} \\ \nabla \cdot \vec{\mathbf{B}} &amp; = 0 \end{split}$$</p>'

try 'latex with \[ .. \]' '\[foo\]' '<p>\[foo\]</p>'
try 'latex with \[ .. \] multi lines' '\[\begin{split}\nabla \times \vec{\mathbf{B}} -\, \frac1c\, \frac{\partial\vec{\mathbf{E}}}{\partial t} & = \frac{4\pi}{c}\vec{\mathbf{j}} \\   \nabla \cdot \vec{\mathbf{E}} & = 4 \pi \rho \\ \nabla \times \vec{\mathbf{E}}\, +\, \frac1c\, \frac{\partial\vec{\mathbf{B}}}{\partial t} & = \vec{\mathbf{0}} \\ \nabla \cdot \vec{\mathbf{B}} & = 0 \end{split}\]' '<p>\[\begin{split}\nabla \times \vec{\mathbf{B}} -\, \frac1c\, \frac{\partial\vec{\mathbf{E}}}{\partial t} &amp; = \frac{4\pi}{c}\vec{\mathbf{j}} \\   \nabla \cdot \vec{\mathbf{E}} &amp; = 4 \pi \rho \\ \nabla \times \vec{\mathbf{E}}\, +\, \frac1c\, \frac{\partial\vec{\mathbf{B}}}{\partial t} &amp; = \vec{\mathbf{0}} \\ \nabla \cdot \vec{\mathbf{B}} &amp; = 0 \end{split}\]</p>'

summary $0
exit $rc

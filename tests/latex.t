. tests/functions.sh

# don't run these tests unless --with-latex
./markdown -V | grep LATEX >/dev/null || exit 0

title "embedded latex"

rc=0
MARKDOWN_FLAGS=

try 'latex w/ \( .. \)' '\(\tex\)' '<p>\(\tex\)</p>'
try 'latex w/ \( .. \) and link inside' '\([(1+2)*3-4](1-2)\)' '<p>\([(1+2)*3-4](1-2)\)</p>'
try 'latex w/ \( .. \) and special characters' 'Equation:\(a<+>b\).' \
    '<p>Equation:\(a&lt;+&gt;b\).</p>'

try 'latex $ delimiter not supported' '$[a](b)$' '<p>$<a href="b">a</a>$</p>'

try 'latex with $$ .. $$' '$$foo$$' '<p>$$foo$$</p>'
try 'latex with $$ .. $$ like a link' '$$[(1+2)*3-4](1-2)$$' '<p>$$[(1+2)*3-4](1-2)$$</p>'
try 'latex with multiple $$ .. $$' '$$[a](b)$$$$[a](b)$$' '<p>$$[a](b)$$$$[a](b)$$</p>'
try 'latex with $$ .. $$ and a real link' '$$[a](b)$$[a](b)$$' '<p>$$[a](b)$$<a href="b">a</a>$$</p>'
try 'latex with $$ .. $$ and a real link' '$$[a](b)$$$[a](b)$$' '<p>$$[a](b)$$$<a href="b">a</a>$$</p>'
try 'latex with $$ .. $$ multi lines' '$$\begin{split}\nabla \times \vec{\mathbf{B}} -\, \frac1c\, \frac{\partial\vec{\mathbf{E}}}{\partial t} & = \frac{4\pi}{c}\vec{\mathbf{j}} \\   \nabla \cdot \vec{\mathbf{E}} & = 4 \pi \rho \\ \nabla \times \vec{\mathbf{E}}\, +\, \frac1c\, \frac{\partial\vec{\mathbf{B}}}{\partial t} & = \vec{\mathbf{0}} \\ \nabla \cdot \vec{\mathbf{B}} & = 0 \end{split}$$' '<p>$$\begin{split}\nabla \times \vec{\mathbf{B}} -\, \frac1c\, \frac{\partial\vec{\mathbf{E}}}{\partial t} &amp; = \frac{4\pi}{c}\vec{\mathbf{j}} \\   \nabla \cdot \vec{\mathbf{E}} &amp; = 4 \pi \rho \\ \nabla \times \vec{\mathbf{E}}\, +\, \frac1c\, \frac{\partial\vec{\mathbf{B}}}{\partial t} &amp; = \vec{\mathbf{0}} \\ \nabla \cdot \vec{\mathbf{B}} &amp; = 0 \end{split}$$</p>'

try 'latex with \[ .. \]' '\[foo\]' '<p>\[foo\]</p>'
try 'latex with \[ .. \] and link inside' '\[[(1+2)*3-4](1-2)\]' '<p>\[[(1+2)*3-4](1-2)\]</p>'
try 'latex with \[ .. \] multi lines' '\[\begin{split}\nabla \times \vec{\mathbf{B}} -\, \frac1c\, \frac{\partial\vec{\mathbf{E}}}{\partial t} & = \frac{4\pi}{c}\vec{\mathbf{j}} \\   \nabla \cdot \vec{\mathbf{E}} & = 4 \pi \rho \\ \nabla \times \vec{\mathbf{E}}\, +\, \frac1c\, \frac{\partial\vec{\mathbf{B}}}{\partial t} & = \vec{\mathbf{0}} \\ \nabla \cdot \vec{\mathbf{B}} & = 0 \end{split}\]' '<p>\[\begin{split}\nabla \times \vec{\mathbf{B}} -\, \frac1c\, \frac{\partial\vec{\mathbf{E}}}{\partial t} &amp; = \frac{4\pi}{c}\vec{\mathbf{j}} \\   \nabla \cdot \vec{\mathbf{E}} &amp; = 4 \pi \rho \\ \nabla \times \vec{\mathbf{E}}\, +\, \frac1c\, \frac{\partial\vec{\mathbf{B}}}{\partial t} &amp; = \vec{\mathbf{0}} \\ \nabla \cdot \vec{\mathbf{B}} &amp; = 0 \end{split}\]</p>'

summary $0
exit $rc

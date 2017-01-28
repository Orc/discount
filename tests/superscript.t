. tests/functions.sh

title "superscript"

rc=0
MARKDOWN_FLAGS=0x0; export MARKDOWN_FLAGS

try -frelax  'A^B -> A<sup>B</sup> (-frelax)' 'A^B' '<p>A<sup>B</sup></p>'
try -fstrict 'A^B != A<sup>B</sup> (-fstrict)' 'A^B' '<p>A^B</p>'
try -fnosuperscript 'A^B != A<sup>B</sup> (-fnosuperscript)' 'A^B' '<p>A^B</p>'
try -frelax 'A^B in link title' '[link](here "A^B")' '<p><a href="here" title="A^B">link</a></p>'
try 'A^(B+2)C^2' 'A^(B+2)C^2' '<p>A<sup>B+2</sup>C<sup>2</sup></p>'
try 'A^((B+2))C^2' 'A^((B+2))C^2' '<p>A<sup>(B+2)</sup>C<sup>2</sup></p>'
try 'A^B+C^D' 'A^B+C^D' '<p>A<sup>B</sup>+C<sup>D</sup></p>'
try 'A^B w/ A in link' '[A](C)^B' '<p><a href="C">A</a><sup>B</sup></p>'
try 'A^B w/ A in html' '<em>A</em>^B' '<p><em>A</em><sup>B</sup></p>'
try 'A^B w/ ^B in link' 'A[^B](C)' '<p>A<a href="C">^B</a></p>'
try 'A^B w/ ^B in incomplete link' 'A[^B]' '<p>A[^B]</p>'

summary $0
exit $rc

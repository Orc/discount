. tests/functions.sh

title "data corruption cases from Fernando Mu√±oz"

rc=0
MARKDOWN_FLAGS=

 try 'id_000002_06' '<p></
 >0' '<p><p>&lt;/</p>

<blockquote><p>0</p></blockquote>'
 try 'id_000009_06' '<pre
 </pr>0' '<p><pre
 </pr>0</p>'
 try 'id_000010_06' '<pre></pr>>' '<p><pre></pr>></p>'
 try 'id_000014_06' '<div></di>-' '<p><div></di>-</p>'
 try 'id_000015_06' '<div></di>>' '<p><div></di>></p>'
 try 'id_000018_06' '<div></di>#0' '<p><div></di>#0</p>'
 try 'id_000029_06' '<div </di>#0' '<p><div </di>#0</p>'
 try 'id_000031_06' '<div </di>>' '<p><div </di>></p>'
 try 'id_000032_06' '<div <di </di></di>*' '<p><div <di </di></di>*</p>'
 try 'id_000038_06' '<p>
    </>00000' '<p><p>
    </>00000</p>'
 try 'id_000039_06' '<dl></d>>' '<p><dl></d>></p>'
 try 'id_000040_06' '<dl></d
	>00000' '<p><dl></d
        >00000</p>'
 try 'id_000043_06' '<dl
<</d>>' '<p><dl
<</d>></p>'
 try 'id_000048_06' '<dl></d
    >00000' '<p><dl></d
    >00000</p>'
 try 'id_000049_06' '<table></tabl>*' '<p><table></tabl>*</p>'
 try 'id_000057_06' '<table></tabl>-' '<p><table></tabl>-</p>'
 try 'id_000062_06' '<table></tabl>+' '<p><table></tabl>+</p>'
 try 'id_000067_06' '<div </di>-' '<p><div </di>-</p>'
 try 'id_000072_06' '<ul></u>-' '<p><ul></u>-</p>'
 try 'id_000074_06' '<ul></u>>' '<p><ul></u>></p>'
 try 'id_000080_06' '<ul
 </u>0' '<p><ul
 </u>0</p>'
 try 'id_000081_06' '<p></
    >00000' '<p><p></
    >00000</p>'
 try 'id_000083_06' '<ul
    </u>00000' '<p><ul
    </u>00000</p>'
 try 'id_000084_06' '<ul></u>*' '<p><ul></u>*</p>'
 try 'id_000089_06' '<p></>*' '<p><p></>*</p>'
 try 'id_000099_06' '<p></>0.' '<p><p></>0.</p>'
 try 'id_000105_06' '<div></di>+' '<p><div></di>+</p>'
 try 'id_000108_06' '<p><!--
    ˇ</>00000' '<p><p>&lt;!&ndash;
    </p>'
 try 'id_000116_06' '<div></di>0.' '<p><div></di>0.</p>'
 try 'id_000119_06' '<div></di
    >00000' '<p><div></di
    >00000</p>'
 try 'id_000120_06' '<!-->>' '<p><!-->></p>'
 try 'id_000128_06' '<p></>#0' '<p><p></>#0</p>'
 try 'id_000130_06' '<blockquote
 </blockquot>0' '<p><blockquote
 </blockquot>0</p>'
 try 'id_000135_06' '<h2></h>>' '<p><h2></h>></p>'
 try 'id_000136_06' '<p 
 </>0' '<p><p
 </>0</p>'
 try 'id_000143_06' '<h2></h>0.' '<p><h2></h>0.</p>'
 try 'id_000148_06' '<p 
    </>00000' '<p><p
    </>00000</p>'
 try 'id_000150_06' '<h2 </h>-' '<p><h2 </h>-</p>'
 try 'id_000152_06' '<p></
	>00000' '<p><p></
        >00000</p>'
 try 'id_000153_06' '<p/
 </>0' '<p><p/
 </>0</p>'
 try 'id_000158_06' '<div </di>+' '<p><div </di>+</p>'
 try 'id_000163_06' '<div></di
	>00000' '<p><div></di
        >00000</p>'
 try 'id_000164_06' '<div></di
 >0' '<p><div>&lt;/di</p>

<blockquote><p>0</p></blockquote>'
 try 'id_000167_06' '<div </di
	>00000' '<p><div </di
        >00000</p>'
 try 'id_000173_06' '<dl></d
 >0' '<p><dl>&lt;/d</p>

<blockquote><p>0</p></blockquote>'
 try 'id_000174_06' '<dl></d>+' '<p><dl></d>+</p>'
 try 'id_000175_06' '<dl></d>=0=
    0' '<p><dl></d>=0=
    0</p>'
 try 'id_000176_06' '<dl
 </d>0' '<p><dl
 </d>0</p>'
 try 'id_000177_06' '<dl
    </d>00000' '<p><dl
    </d>00000</p>'
 try 'id_000182_06' '<p></>-' '<p><p></>-</p>'
 try 'id_000189_06' '<ol></o>#0' '<p><ol></o>#0</p>'
 try 'id_000190_06' '<p/</>*' '<p>&lt;p/</>*</p>'
 try 'id_000192_06' '<div </di>*' '<p><div </di>*</p>'
 try 'id_000197_06' '<ul></u
 >0' '<p><ul>&lt;/u</p>

<blockquote><p>0</p></blockquote>'
 try 'id_000201_06' '<ul></u>+' '<p><ul></u>+</p>'
 try 'id_000203_06' '<ul></u
    >00000' '<p><ul></u
    >00000</p>'
 try 'id_000209_06' '<!--
    -->00000' '<p><!--
    -->00000</p>'
 try 'id_000211_06' '<p></>+' '<p><p></>+</p>'
 try 'id_000219_06' '<!-->*' '<p><!-->*</p>'
 try 'id_000222_06' '<p/</>-' '<p>&lt;p/</>-</p>'
 try 'id_000223_06' '<!--
	-->00000' '<p><!--
        -->00000</p>'
 try 'id_000224_06' '<!-->0.' '<p><!-->0.</p>'
 try 'id_000226_06' '<p>
 </>0' '<p><p>
 </>0</p>'
 try 'id_000228_06' '<div></di>*' '<p><div></di>*</p>'
 try 'id_000231_06' '<div
 </di>0' '<p><div
 </di>0</p>'
 try 'id_000238_06' '<p></>[]' '<p><p></>[]</p>'
 try 'id_000240_06' '<h2
 </h>0' '<p><h2
 </h>0</p>'
 try 'id_000243_06' '<p </>>' '<p><p </>></p>'
 try 'id_000246_06' '<p></>>' '<p><p></>></p>'
 try 'id_000248_06' '<p </>[]' '<p><p </>[]</p>'

summary $0
exit $rc 

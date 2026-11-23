"""Build the role icons from the original BI. Letter badges are below the symbol."""
from pathlib import Path
from io import BytesIO
import struct
import fitz
from PIL import Image, ImageDraw
root=Path(__file__).resolve().parents[1]/'brand'
with fitz.open(root/'symbol.svg') as doc:
    pix=doc[0].get_pixmap(matrix=fitz.Matrix(2,2), alpha=True)
symbol=Image.open(BytesIO(pix.tobytes('png'))).convert('RGBA')
symbol=symbol.crop(symbol.getbbox())
sizes=(16,20,24,32,40,48,64,128,256)
# Rounded, font-independent letter outlines below the unchanged BI.
glyphs={'SERVER':'M77 23 C63 8 27 9 22 30 C18 46 36 48 50 51 C65 54 81 58 77 72 C72 91 38 94 20 76',
        'USER':'M23 16 L23 61 C23 95 77 95 77 61 L77 16'}
def badge_image(role,fg):
    svg=(f'<svg xmlns="http://www.w3.org/2000/svg" width="100" height="100"><path d="{glyphs[role]}" fill="none" stroke="{fg}" stroke-width="15" stroke-linecap="round"/></svg>').encode()
    with fitz.open(stream=svg,filetype='svg') as doc:
        pix=doc[0].get_pixmap(matrix=fitz.Matrix(7,7),alpha=True)
    return Image.open(BytesIO(pix.tobytes('png'))).convert('RGBA')
letters={r:badge_image(r,'#FFFFFF' if r=='SERVER' else '#14263D') for r in glyphs}
def render(role,side):
    scale=12; N=side*scale
    im=Image.new('RGBA',(N,N));d=ImageDraw.Draw(im)
    margin=round(N*.035); corner=round(N*.12); badge=round(N*.60)
    d.rounded_rectangle((margin,margin,N-margin-1,N-margin-1),radius=corner,fill='white')
    art=symbol.copy();art.thumbnail((round(N*.76),round(N*.51)),Image.Resampling.LANCZOS)
    im.alpha_composite(art,((N-art.width)//2,(badge-art.height)//2+round(N*.01)))
    color='#14263D' if role=='SERVER' else '#F27038'
    fg='white' if role=='SERVER' else '#14263D'
    d.rounded_rectangle((margin,badge,N-margin-1,N-margin-1),radius=corner,fill=color)
    d.rectangle((margin,badge,N-margin-1,badge+corner),fill=color)
    h=round((N-margin-badge)*.92);letter=letters[role].resize((h,h),Image.Resampling.LANCZOS)
    im.alpha_composite(letter,((N-h)//2,badge+(N-margin-badge-h)//2))
    return im.resize((side,side),Image.Resampling.LANCZOS)
for role in glyphs:
    entries=[];body=bytearray()
    for side in sizes:
        im=render(role,side)
        if side>=128:
            b=BytesIO(); im.save(b,format='PNG',optimize=True);data=b.getvalue()
        else:
            rows=im.transpose(Image.Transpose.FLIP_TOP_BOTTOM);xor=rows.tobytes('raw','BGRA')
            stride=((side+31)//32)*4; mask=bytearray(side*stride)
            for y in range(side):
                for x in range(side):
                    if rows.getpixel((x,y))[3]==0:mask[y*stride+x//8]|=128>>(x%8)
            data=struct.pack('<IiiHHIIIIII',40,side,2*side,1,32,0,len(xor)+len(mask),0,0,0,0)+xor+mask
        entries.append(struct.pack('<BBBBHHII',side%256,side%256,0,0,1,32,len(data),6+16*len(sizes)+len(body)))
        body.extend(data)
    (root/(role+'.ico')).write_bytes(struct.pack('<HHH',0,1,len(sizes))+b''.join(entries)+body)
preview=Image.new('RGBA',(200,92),'#F1F3F7')
for role,x in [('SERVER',12),('USER',112)]:
    preview.alpha_composite(render(role,76),(x,8))
preview.save(root/'preview.png', optimize=True)

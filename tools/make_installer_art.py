"""Erzeugt die Raben-Grafiken für beide Installer (Windows-Wizard, .ico, macOS-Hintergrund).
Benötigt:  pip install pillow numpy      Aufruf:  python tools/make_installer_art.py
"""
import os
import random
import numpy as np
from PIL import Image, ImageDraw, ImageFont

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), ".."))
SS = 4  # Supersampling für glatte Kanten

NIGHT = (11, 10, 18)
INDIGO = (27, 24, 56)
VIOLET = (139, 108, 255)
SILVER = (201, 198, 221)
GOLD = (255, 194, 71)
DARK = (7, 6, 13)


def sky(w, h, moon=None, moon_r=0, seed=7):
    t = np.linspace(0, 1, h)[:, None, None]
    top, bot = np.array(INDIGO, float), np.array(NIGHT, float)
    img = top * (1 - t) + bot * t
    img = np.repeat(img, w, axis=1)
    if moon:
        yy, xx = np.mgrid[0:h, 0:w]
        d = np.sqrt((xx - moon[0]) ** 2 + (yy - moon[1]) ** 2)
        glow = np.clip(1 - d / (moon_r * 4.2), 0, 1) ** 2.2 * 0.55
        img = img * (1 - glow[..., None]) + np.array(SILVER, float) * glow[..., None]
    im = Image.fromarray(img.astype("uint8"), "RGB").convert("RGBA")
    rnd = random.Random(seed)
    d = ImageDraw.Draw(im)
    for _ in range(w * h // 1800):
        x, y = rnd.randrange(w), rnd.randrange(h)
        a = rnd.randrange(60, 170)
        d.point((x, y), fill=(255, 255, 255, a))
    if moon:
        d.ellipse([moon[0] - moon_r, moon[1] - moon_r, moon[0] + moon_r, moon[1] + moon_r], fill=(233, 229, 255, 240))
        for dx, dy, r in ((-0.35, -0.3, 0.26), (0.25, 0.25, 0.18)):
            cx, cy, rr = moon[0] + dx * moon_r, moon[1] + dy * moon_r, r * moon_r
            d.ellipse([cx - rr, cy - rr, cx + rr, cy + rr], fill=(186, 180, 220, 140))
    return im


def bez(p0, p1, p2, n=18):
    return [((1 - t) ** 2 * p0[0] + 2 * (1 - t) * t * p1[0] + t * t * p2[0],
             (1 - t) ** 2 * p0[1] + 2 * (1 - t) * t * p1[1] + t * t * p2[1]) for t in (i / n for i in range(n + 1))]


def wing(flap, length):
    tip_y = -flap * length * 0.95 - 2
    pts = bez((6, -3), (2, -flap * length * 0.6 - 10), (-length * 0.35, tip_y))
    pts += bez((-length * 0.35, tip_y), (-length * 0.5, -flap * length * 0.2 + 7), (-9, 2))[1:]
    return pts


def ellipse_pts(cx, cy, rx, ry, n=40):
    import math
    return [(cx + rx * math.cos(a * 2 * math.pi / n), cy + ry * math.sin(a * 2 * math.pi / n)) for a in range(n)]


def raven(im, x, y, scale, flap=0.6, alpha=255):
    """Zeichnet einen Raben (schaut nach rechts) auf ein RGBA-Bild."""
    w, h = im.size
    layer = Image.new("RGBA", (w * SS, h * SS), (0, 0, 0, 0))
    d = ImageDraw.Draw(layer)

    def tf(pts):
        return [((x + px * scale) * SS, (y + py * scale) * SS) for px, py in pts]

    shapes = [
        (wing(max(-1, min(1, flap - 0.4)), 30), 0.65),
        ([(-12, -2), (-29, -5), (-26, 3.5)], 1),
        (ellipse_pts(0, 0, 14, 5), 1),
        (ellipse_pts(15, -4, 5, 4.5), 1),
        ([(18.5, -6.5), (29, -3), (18.5, -1.5)], 1),
        (wing(flap, 34), 1),
    ]
    for pts, a in shapes:
        p = tf(pts)
        d.polygon(p, fill=DARK + (int(alpha * a),), outline=VIOLET + (int(90 * a),))
    ex, ey = (x + 16.2 * scale) * SS, (y - 4.6 * scale) * SS
    r = max(1.0, 1.0 * scale) * SS
    d.ellipse([ex - r, ey - r, ex + r, ey + r], fill=GOLD + (255,))
    layer = layer.resize((w, h), Image.LANCZOS)
    im.alpha_composite(layer)


def font(size, bold=True):
    for p in ("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf" if bold else "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
              "C:/Windows/Fonts/segoeuib.ttf", "/System/Library/Fonts/Helvetica.ttc"):
        if os.path.exists(p):
            return ImageFont.truetype(p, size)
    return ImageFont.load_default(size)


def spaced_text(d, xy, text, fnt, fill, spacing=2):
    x, y = xy
    for ch in text:
        d.text((x, y), ch, font=fnt, fill=fill)
        x += d.textlength(ch, font=fnt) + spacing


def wizard():
    w, h = 164, 314
    im = sky(w, h, moon=(128, 92), moon_r=16)
    raven(im, 78, 150, 2.1, flap=0.7)
    raven(im, 50, 232, 1.2, flap=-0.4, alpha=210)
    raven(im, 124, 270, 0.9, flap=0.2, alpha=170)
    d = ImageDraw.Draw(im)
    spaced_text(d, (14, 20), "RAVEN", font(15), SILVER + (255,), 3)
    spaced_text(d, (14, 40), "CLIPPER", font(15), VIOLET + (255,), 3)
    im.convert("RGB").save(os.path.join(ROOT, "assets", "wizard.bmp"))

    s = Image.new("RGBA", (110, 110), NIGHT + (255,))
    s.alpha_composite(sky(110, 110, moon=(80, 30), moon_r=10, seed=3))
    raven(s, 66, 62, 1.9, flap=0.6)
    s = s.resize((55, 55), Image.LANCZOS)
    s.convert("RGB").save(os.path.join(ROOT, "assets", "wizard_small.bmp"))

    ico = sky(256, 256, moon=(190, 62), moon_r=26)
    raven(ico, 118, 140, 4.4, flap=0.6)
    ico.save(os.path.join(ROOT, "assets", "icon.ico"), sizes=[(16, 16), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)])


def mac_background():
    w, h = 1240, 836  # @2x von 620x418
    im = sky(w, h, moon=(1000, 170), moon_r=62)
    raven(im, 300, 560, 6.0, flap=0.7)
    raven(im, 700, 380, 3.2, flap=-0.5, alpha=220)
    raven(im, 880, 640, 2.4, flap=0.3, alpha=190)
    d = ImageDraw.Draw(im)
    spaced_text(d, (56, 60), "RAVENCLIPPER", font(54), SILVER + (255,), 6)
    spaced_text(d, (60, 130), "SOFT CLIPPER FOR BEATS", font(24), VIOLET + (255,), 5)
    out = os.path.join(ROOT, "installer", "mac", "resources", "background.png")
    im.save(out)


if __name__ == "__main__":
    os.makedirs(os.path.join(ROOT, "assets"), exist_ok=True)
    os.makedirs(os.path.join(ROOT, "installer", "mac", "resources"), exist_ok=True)
    wizard()
    mac_background()
    print("Installer-Grafiken erzeugt.")

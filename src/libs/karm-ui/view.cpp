#include "view.h"

namespace Karm::Ui {

/* --- Text ----------------------------------------------------------------- */

Strong<Media::Fontface> regularFontface() {
    static Strong<Media::Fontface> f = []() {
        return Media::loadFontface("res/fonts/inter/Inter-Regular.ttf").unwrap();
    }();
    return f;
}

Strong<Media::Fontface> mediumFontface() {
    static Strong<Media::Fontface> f = []() {
        return Media::loadFontface("res/fonts/inter/Inter-Medium.ttf").unwrap();
    }();
    return f;
}

Strong<Media::Fontface> boldFontface() {
    static Strong<Media::Fontface> f = []() {
        return Media::loadFontface("res/fonts/inter/Inter-Bold.ttf").unwrap();
    }();
    return f;
}

Strong<Media::Fontface> italicFontface() {
    static Strong<Media::Fontface> f = []() {
        return Media::loadFontface("res/fonts/inter/Inter-Italic.ttf").unwrap();
    }();
    return f;
}

TextStyle TextStyle::displayLarge() {
    return {
        .font = Media::Font{
            regularFontface(),
            57,
        },
    };
}

TextStyle TextStyle::displayMedium() {
    return {
        .font = Media::Font{
            regularFontface(),
            45,
        },
    };
}

TextStyle TextStyle::displaySmall() {
    return {
        .font = Media::Font{
            regularFontface(),
            36,
        },
    };
}

TextStyle TextStyle::headlineLarge() {
    return {
        .font = Media::Font{
            regularFontface(),
            32,
        },
    };
}

TextStyle TextStyle::headlineMedium() {
    return {
        .font = Media::Font{
            regularFontface(),
            28,
        },
    };
}

TextStyle TextStyle::headlineSmall() {
    return {
        .font = Media::Font{
            regularFontface(),
            24,
        },
    };
}

TextStyle TextStyle::titleLarge() {
    return {
        .font = Media::Font{
            regularFontface(),
            22,
        },
    };
}

TextStyle TextStyle::titleMedium() {
    return {
        .font = Media::Font{
            mediumFontface(),
            16,
        },
    };
}

TextStyle TextStyle::titleSmall() {
    return {
        .font = Media::Font{
            mediumFontface(),
            14,
        },
    };
}

TextStyle TextStyle::labelLarge() {
    return {
        .font = Media::Font{
            mediumFontface(),
            14,
        },
    };
}
TextStyle TextStyle::labelMedium() {
    return {
        .font = Media::Font{
            mediumFontface(),
            12,
        },
    };
}
TextStyle TextStyle::labelSmall() {
    return {
        .font = Media::Font{
            mediumFontface(),
            11,
        },
    };
}

TextStyle TextStyle::bodyLarge() {
    return {
        .font = Media::Font{
            regularFontface(),
            16,
        },
    };
}

TextStyle TextStyle::bodyMedium() {
    return {
        .font = Media::Font{
            regularFontface(),
            14,
        },
    };
}
TextStyle TextStyle::bodySmall() {
    return {
        .font = Media::Font{
            regularFontface(),
            12,
        },
    };
}

struct Text : public View<Text> {
    TextStyle _style;
    String _text;
    Opt<Media::FontMesure> _mesure;

    Text(TextStyle style, String text)
        : _style(style), _text(text) {}

    void reconcile(Text &o) override {
        _text = o._text;
        _mesure = NONE;
    }

    Media::FontMesure mesure() {
        if (_mesure) {
            return *_mesure;
        }
        _mesure = _style.font.mesureStr(_text);
        return *_mesure;
    }

    void paint(Gfx::Context &g, Math::Recti) override {
        g.save();

        auto m = mesure();
        auto baseline = bound().topStart() + m.baseline.cast<int>();

        if (_style.color) {
            g.fillStyle(*_style.color);
        }

        g.textFont(_style.font);
        g.fill(baseline, _text);

        if (debugShowLayoutBounds) {
            g._line(
                {
                    bound().topStart() + m.baseline.cast<int>(),
                    bound().topEnd() + m.baseline.cast<int>(),
                },
                Gfx::PINK);
            g._rect(bound(), Gfx::CYAN);
        }

        g.restore();
    }

    Math::Vec2i size(Math::Vec2i, Layout::Hint) override {
        return mesure().linebound.size().cast<int>();
    }
};

Child text(TextStyle style, Str text) {
    return makeStrong<Text>(style, text);
}

Child text(Str text) {
    return makeStrong<Text>(TextStyle::labelMedium(), text);
}

/* --- Icon ----------------------------------------------------------------- */

struct Icon : public View<Icon> {
    Media::Icon _icon;
    Opt<Gfx::Color> _color;

    Icon(Media::Icon icon, Opt<Gfx::Color> color)
        : _icon(icon), _color(color) {}

    void reconcile(Icon &o) override {
        _icon = o._icon;
    }

    void paint(Gfx::Context &g, Math::Recti) override {
        g.save();
        if (_color)
            g.fillStyle(_color.unwrap());
        g.fill(bound().topStart(), _icon);
        if (debugShowLayoutBounds)
            g._rect(bound(), Gfx::CYAN);
        g.restore();
    }

    Math::Vec2i size(Math::Vec2i, Layout::Hint) override {
        return _icon.bound().size().cast<int>();
    }
};

Child icon(Media::Icon icon, Opt<Gfx::Color> color) {
    return makeStrong<Icon>(icon, color);
}

Child icon(Media::Icons i, double size, Opt<Gfx::Color> color) {
    return icon(Media::Icon{i, size}, color);
}

/* --- Image ---------------------------------------------------------------- */

struct Image : public View<Image> {
    Media::Image _image;

    Image(Media::Image image)
        : _image(image) {
    }

    void paint(Gfx::Context &g, Math::Recti) override {
        g.blit(bound(), _image);
        if (debugShowLayoutBounds)
            g._rect(bound(), Gfx::CYAN);
    }

    Math::Vec2i size(Math::Vec2i, Layout::Hint) override {
        return _image.bound().size().cast<int>();
    }
};

Child image(Media::Image image) {
    return makeStrong<Image>(image);
}

/* --- Canvas --------------------------------------------------------------- */

struct Canvas : public View<Canvas> {
    OnPaint _onPaint;

    Canvas(OnPaint onPaint)
        : _onPaint(std::move(onPaint)) {}

    void reconcile(Canvas &o) override {
        _onPaint = std::move(o._onPaint);
        View<Canvas>::reconcile(o);
    }

    void paint(Gfx::Context &g, Math::Recti) override {
        g.save();
        g.clip(_bound);
        g.origin(_bound.xy);
        _onPaint(g, _bound.wh);
        g.restore();
    }

    Math::Vec2i size(Math::Vec2i, Layout::Hint hint) override {
        if (hint == Layout::Hint::MIN) {
            return 0;
        } else {
            return _bound.wh;
        }
    }
};

Child canvas(OnPaint onPaint) {
    return makeStrong<Canvas>(std::move(onPaint));
}

/* --- Blur ----------------------------------------------------------------- */

struct Blur : public ProxyNode<Blur> {
    int _radius;

    Blur(int radius, Child child)
        : ProxyNode<Blur>(std::move(child)), _radius(radius) {}

    void reconcile(Blur &o) override {
        _radius = o._radius;
        ProxyNode<Blur>::reconcile(o);
    }

    void paint(Gfx::Context &g, Math::Recti r) override {
        g.blur(bound(), _radius);
        ProxyNode<Blur>::paint(g, r);
    }
};

Child blur(int radius, Child child) {
    return makeStrong<Blur>(radius, std::move(child));
}

} // namespace Karm::Ui

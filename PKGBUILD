pkgname=site-blocker
pkgver=1.0
pkgrel=1
pkgdesc="Simple website blocker with GUI"
arch=('x86_64')
url="https://github.com/yourname/site-blocker"
license=('MIT')
depends=('gtk3')
makedepends=('gcc' 'make' 'pkg-config')
install="${pkgname}.install"
source=("${pkgname}-${pkgver}.tar.gz"
        "${pkgname}.desktop"
        "${pkgname}.png"
        "${pkgname}.policy")
sha256sums=('SKIP'
            'SKIP'
            'SKIP'
            'SKIP')

build() {
    cd "${srcdir}/${pkgname}-${pkgver}"
    make
}

package() {
    cd "${srcdir}/${pkgname}-${pkgver}"
    
    install -Dm755 "${pkgname}" "${pkgdir}/usr/bin/${pkgname}"
    install -Dm644 "${srcdir}/${pkgname}.desktop" "${pkgdir}/usr/share/applications/${pkgname}.desktop"
    install -Dm644 "${srcdir}/${pkgname}.png" "${pkgdir}/usr/share/pixmaps/${pkgname}.png"
    install -Dm644 "${srcdir}/${pkgname}.policy" "${pkgdir}/usr/share/polkit-1/actions/com.${pkgname}.policy"
    install -dm755 "${pkgdir}/etc/${pkgname}"
}

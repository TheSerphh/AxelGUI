# Maintainer: TheSerphh <souravgope765@gmail.com>
pkgname=axel-gui
pkgver=1.0.0
pkgrel=1
pkgdesc="Modern Qt6 GUI wrapper for the Axel download accelerator with Firefox integration"
arch=('x86_64')
url="https://github.com/theserphh/AxelGUI"
license=('GPL-3.0-or-later')
depends=('qt6-base' 'axel' 'python')
makedepends=('clang' 'cmake' 'ninja')
source=("$pkgname-$pkgver.tar.gz::$url/archive/v$pkgver.tar.gz")
sha256sums=('SKIP')

build() {
    cd "$srcdir/$pkgname-$pkgver"
    export CC=clang
    export CXX=clang++

    cmake -B build -GNinja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    ninja -C build
}

package() {
    cd "$srcdir/$pkgname-$pkgver"
    DESTDIR="$pkgdir" ninja -C build install
}

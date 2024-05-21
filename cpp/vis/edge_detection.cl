#define H 720u
#define W 720u
#define WORKGROUP_SIZE 18u

global uint img[H][W];
global uint edges[H][W];

static uint dif(uint a, uint b) {
    return a > b ? a - b : b - a;
}

static bool compareColours(uint a, uint b) {
    return dif(a >> 16, b >> 16) > 9u ||
           dif((a >> 8) & 0xFFu, (b >> 8) & 0xFFu) > 9u ||
           dif(a & 0xFFu, b & 0xFFu) > 9u;
}

kernel __attribute__((work_group_size_hint(WORKGROUP_SIZE, WORKGROUP_SIZE, 1)))
void detect() {
    uint x = get_global_id(0u), y = get_global_id(1u);
    if (x >= W || y >= H) return;

    edges[y][x] = 0u;
    bool nt = y != 0, nb = y != H - 1,
            nl = x != 0, nr = x != W - 1;
    if (nt && compareColours(img[y][x], img[y - 1][x])) // top
        edges[y][x] |= 1u;
    if (nt && nr && compareColours(img[y][x], img[y - 1][x + 1])) // top-right
        edges[y][x] |= 2u;
    if (nr && compareColours(img[y][x], img[y][x + 1])) // right
        edges[y][x] |= 4u;
    if (nb && nr && compareColours(img[y][x], img[y + 1][x + 1])) // bottom-right
        edges[y][x] |= 8u;
    if (nb && compareColours(img[y][x], img[y + 1][x])) // bottom
        edges[y][x] |= 16u;
    if (nb && nl && compareColours(img[y][x], img[y + 1][x - 1])) // bottom-left
        edges[y][x] |= 32u;
    if (nl && compareColours(img[y][x], img[y][x - 1])) // left
        edges[y][x] |= 64u;
    if (nt && nl && compareColours(img[y][x], img[y - 1][x - 1])) // top-left
        edges[y][x] |= 128u;
}

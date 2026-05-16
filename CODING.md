### Note to self

Because OpenStreetMap tiles stream securely via HTTPS, make sure the OpenSSL binaries (libcrypto and libssl) are present in your Windows deployment directory alongside your executable, otherwise map tiles will load as blank gray grids.

### example of cpt loading

```
    Cpt *myCpt = Cpt::fromGef(filePath);
    // Always check if the pointer is null (meaning parsing failed or file couldn't open)
    if (!myCpt) {
        qWarning() << "Failed to parse GEF file.";
        return;
```
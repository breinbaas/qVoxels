
### example of cpt loading

```
    Cpt *myCpt = Cpt::fromGef(filePath);
    // Always check if the pointer is null (meaning parsing failed or file couldn't open)
    if (!myCpt) {
        qWarning() << "Failed to parse GEF file.";
        return;
```
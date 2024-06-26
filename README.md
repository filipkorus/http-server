## HTTP server
author: [Filip Korus](https://github.com/filipkorus/)

### Functionalities
- Every request is handled on a separate thread
- Serving files from pre-defined directory (DOCUMENT_ROOT in [common.h](common.h))
- Parsing headers of HTTP request
- Directory browsing (for those directories where index.html file is not present)
- Reading body fields from POST requests and sending them back to client (JSON and x-www-form-urlencoded formats are supported) - demo page available on [http://localhost:8080/post/](www/post/index.html)

### Compile
```bash
make
```

### Run
```bash
make run
```

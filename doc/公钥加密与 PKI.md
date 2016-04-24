# 公钥加密与 PKI
<center>康荣 2015311965</center>
## 一、实验描述
本实验的学习目的是让学生熟悉公钥加密与 PKI 的概念，本课实验包含公钥加密，数字签名，公钥认证，认证授权，基于 PKI 授权等内容。学生将以使用工具或者写程序的方式建立基于 PKI 的安全信道。

## 二、实验环境
由于实验楼的网速问题，操作起来很不方便，因此本次的实验在ubuntu 12.04 desktop版上完成。

操作系统：ubuntu 12.04 desktop；

内存：4GB。

## 三、实验内容
### 实验 1:成为数字证书认证机构（CA）
数字证书的作用是证明证书中列出的用户合法拥有证书中列出的公开密钥。CA 机构的数字签名使得攻击者不能伪造和篡改证书。它负责产生、分配并管理所有参与网上交易的个体所需的数字证书，因此是安全电子交易的核心环节。

本次的实验其实是自己编写证书，自己给自己加密，授权，然后自己启动一个服务器，测试请求是否可以通过验证。

在第一步，首先生成证书：

按照教程，首先拷贝一份openssl.conf作为配置文件，在openssl.conf同目录下按照conf的要求添加文件：

```
dir = ./demoCA # Where everything is kept
certs = $dir/certs # Where the issued certs are kept
crl_dir = $dir/crl # Where the issued crl are kept
new_certs_dir = $dir/newcerts # default place for new certs.
database = $dir/index.txt # database index file.
serial = $dir/serial # The current serial number
```

文件目录树如下：

![](http://i2.piimg.com/da0a817c4950c829.png)

运行命令：
>openssl req -new -x509 -keyout ca.key -out ca.crt -config openssl.cnf

这一步会生成ca.key与ca.crt两个文件，ca.key包括CA的私钥,而ca.crt包含了公钥证书：

![](http://i2.piimg.com/a0d3eea555583a39.png)

这一步应注意，ca生成需要添加密码。实验一完成。

### 实验 2: 为 PKILabServer.com 生成证书
**Step 1**：生成公开/私有密钥对

第一步，首先需要生成它自己的公开/私有密钥对。运行以下命令来生成 RSA 密钥对。你同时需要提供一个密码来保护你的密钥。密钥会被保存在server.key文件中。

> openssl genrsa -des3 -out server.key 1024

**Step 2**: 生成签名请求

一旦公司拥有了密钥文件，它应当生成证书签名请求（csr文件）。csr将被发送给CA，CA会为该请求生成证书（通常在确认csr中的身份信息匹配后）。

> openssl req -new -key server.key -out server.csr -config openssl.cnf

这一步的产生过程中一定要输入common name，不能默认为空。这个很容易理解，如果一个证书连访问域名都不指定，签名机构当然无法进行签名和验证。如果一路按回车默认的话，产生文件将提示如下：

![](http://i2.piimg.com/4305b8ceccdc79de.png)

这将在下一步产生错误。

**Step 3**: 生成证书 生成证书。csr文件需要拥有CA的签名来构成证书。在现实世界中，csr文件常常被发送给可信任的CA签名。本实验中，我们将使用我们自己的CA来生成证书：

> openssl ca -in server.csr -out server.crt -cert ca.crt -keyfile ca.key -config openssl.cnf


### 实验 3：在网站中使用 PKI
这一步是查看签发的证书如何保证安全以及识别证书。首先将签名好的证书与私钥连城pem，然后，`openssl s_server -cert server.pem -www`是以server.pem的私钥和签名信息为基础启动的网络服务器。

```
# Combine the secret key and certificate into one file
$ cp server.key server.pem
$ cat server.crt >> server.pem
# Launch the web server using server.pem
$ openssl s_server -cert server.pem -www
```

如果上一步没有指明common name，则会报错：

![](http://i2.piimg.com/4d340bb46b7c37b3.png)

查看server.pem会发现，server.crt是空的，也即没有任何加密过。

指定了common name之后，证书终于生成了：

![](http://i2.piimg.com/f72a5016da2a78e4.png)

然后发送访问请求。这里我的common name是server，因此访问https://server:4433。

![](http://i3.piimg.com/e1814013e8e61a12.png)

访问并不正确，在Firefox中导入自己的CA证书，重新访问，已经可以了：

![](http://i3.piimg.com/7042389a1e5569f9.png)

由以上的配置可知，签发证书只会针对于特定的域名。显然，如果用localhost访问127的话，网络代理根本不会识别。实验结果证明了这一点。

### 实验 4：使用 PKI 与 PKILabServer.com 建立安全 TCP 链接
这一步实验楼所提供的源代码编译不通过，因此找到了另一份demo代码来演示openssl建立TCP的过程(只展现流程，因此去掉了许多参数处理和数据处理)，效果是，客户端连接服务器，服务器发送`server->client`，客户端返回`from server->client`

服务器端流程如下：

```
    /* SSL 库初始化 */
    SSL_library_init();
    /* 载入所有 SSL 算法 */
    OpenSSL_add_all_algorithms();
    /* 载入所有 SSL 错误消息 */
    SSL_load_error_strings();
    /* 以 SSL V2 和 V3 标准兼容方式产生一个 SSL_CTX ，即 SSL Content Text */
    ctx = SSL_CTX_new(SSLv23_server_method());

    /* 载入用户的数字证书， 此证书用来发送给客户端。 证书里包含有公钥 */
    SSL_CTX_use_certificate_file(ctx, argv[4], SSL_FILETYPE_PEM)
    /* 载入用户私钥 */
    (SSL_CTX_use_PrivateKey_file(ctx, argv[5], SSL_FILETYPE_PEM)
    /* 检查用户私钥是否正确 */
	 SSL_CTX_check_private_key(ctx))

    /* 开启一个 socket 监听 */
    sockfd = socket(PF_INET, SOCK_STREAM, 0)
    /* 监听指定端口和地址 */
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = PF_INET;
    my_addr.sin_port = htons(myport);
    if (argv[3])
        my_addr.sin_addr.s_addr = inet_addr(argv[3]);
    else
        my_addr.sin_addr.s_addr = INADDR_ANY;
	
	 /* bind 和listen */
    bind(sockfd, (struct sockaddr *) &my_addr, sizeof(struct sockaddr))

    listen(sockfd, lisnum)
    
    /* 等待client的连接 */
    while (1) {
        SSL *ssl;
        len = sizeof(struct sockaddr);
        /* 等待客户端连上来 */
        new_fd = accept(sockfd, (struct sockaddr *) &their_addr, &len)
        /* 基于 ctx 产生一个新的 SSL */
        ssl = SSL_new(ctx);
        /* 将连接用户的 socket 加入到 SSL */
        SSL_set_fd(ssl, new_fd);
        /* 建立 SSL 连接 */
        SSL_accept(ssl);

        /* 开始处理每个新连接上的数据收发 */
        bzero(buf, MAXBUF + 1);
        strcpy(buf, "server->client");
        /* 发消息给客户端 */
        len = SSL_write(ssl, buf, strlen(buf));

        bzero(buf, MAXBUF + 1);
        /* 接收客户端的消息 */
        len = SSL_read(ssl, buf, MAXBUF);
        printf("msg receive failed: %d error code: %s \n",
                 errno, strerror(errno));
        /* 处理每个新连接上的数据收发结束 */
      finish:
        /* 关闭 SSL 连接 */
        SSL_shutdown(ssl);
        /* 释放 SSL */
        SSL_free(ssl);
        /* 关闭 socket */
        close(new_fd);
    }

    /* 关闭监听的 socket */
    close(sockfd);
    /* 释放 CTX */
    SSL_CTX_free(ctx);
```

客户端流程如下：

```

    /* SSL库初始化 */
    SSL_library_init();	
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());

    /* 创建一个 socket 用于 tcp 通信 */
    sockfd = socket(AF_INET, SOCK_STREAM, 0)

    /* 初始化服务器端（对方）的地址和端口信息 */
    bzero(&dest, sizeof(dest));
    dest.sin_family = AF_INET;
    dest.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], (struct in_addr *) &dest.sin_addr.s_addr)

    /* 连接服务器 */
    connect(sockfd, (struct sockaddr *) &dest, sizeof(dest))

    /* 基于 ctx 产生一个新的 SSL */
    ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sockfd);
    /* 建立 SSL 连接 */
    SSL_connect(ssl)

    /* 接收对方发过来的消息，最多接收 MAXBUF 个字节 */
    bzero(buffer, MAXBUF + 1);
    /* 接收服务器来的消息 */
    len = SSL_read(ssl, buffer, MAXBUF);
    printf("receive msg success:%s total byte:%d\n",
               buffer, len);
    /* 发消息给服务器 */
    len = SSL_write(ssl, buffer, strlen(buffer));
    finish:
    /* 关闭连接 */
    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sockfd);
    SSL_CTX_free(ctx);
    return 0;
```

实验结果如下：

客户端：
![](http://i2.piimg.com/19cfe17c854b2d5d.png)

服务器端：

![](http://i2.piimg.com/60be14e505bc85f0.png)
### 实验 5：性能比较 RSA vs AES
运行基准测试：

>openssl speed rsa
>
>openssl speed aes

结果如下：

|type |byte| sign |   verify|    sign/s| verify/s|
|---|---|---|---|---|---|
|rsa | 512| bits| 0.000053s| 0.000005s|  18816.3| 217718.5
|rsa |1024| bits| 0.000169s| 0.000011s|   5931.6|  87103.4
|rsa |2048| bits| 0.001315s| 0.000039s|    760.4|  25595.7
|rsa |4096| bits| 0.009619s| 0.000144s|    104.0|   6946.5

|type|16 bytes|64 bytes|256 bytes|1024 bytes|8192 bytes|
|---|---|---|---|---|---|
|aes-128 cbc|122944.71k|136610.89k|138174.01k|282696.41k|290292.25k|
|aes-192 cbc|99381.34k|108854.57k|105888.64k|250038.14k|252969.63k|
|aes-256 cbc|87801.50k|96759.67k|99415.35k|218840.50k|220203.65k|

### 实验 6：创建数字签名
这一步按照教程，完成了数字签名实验。

数字签名原理如下：

数字签名的文件的完整性是很容易验证的（不需要骑缝章，骑缝签名，也不需要笔迹专家），而且数字签名具有不可抵赖性（不需要笔迹专家来验证）。

简单地说,所谓数字签名就是附加在数据单元上的一些数据,或是对数据单元所作的密码变换。这种数据或变换允许数据单元的接收者用以确认数据单元的来源和数据单元的完整性并保护数据,防止被人(例如接收者)进行伪造。它是对电子形式的消息进行签名的一种方法,一个签名消息能在一个通信网络中传输。基于公钥密码体制和私钥密码体制都可以获得数字签名,主要是基于公钥密码体制的数字签名。包括普通数字签名和特殊数字签名。普通数字签名算法有RSA、ElGamal、Fiat-Shamir、Guillou- Quisquarter、Schnorr、Ong-Schnorr-Shamir数字签名算法、Des/DSA,椭圆曲线数字签名算法和有限自动机数字签名算法等。特殊数字签名有盲签名、代理签名、群签名、不可否认签名、公平盲签名、门限签名、具有消息恢复功能的签名等,它与具体应用环境密切相关。显然,数字签名的应用涉及到法律问题,美国联邦政府基于有限域上的离散对数问题制定了自己的数字签名标准(DSS)。

试验验证如下：

> openssl genrsa -des3 -out myrsaCA.pem 1024

生成的pem文件：

![](http://i4.piimg.com/1f6f573409c45c28.png)

> openssl rsa -in myrsaCA.pem -pubout -out myrsapubkey.pem
 
生成公钥文件：

![](http://i4.piimg.com/6b5637c2e36f4e49.png)
 
> openssl dgst -sha256 -out example.sha256 -sign myrsaCA.pem example.txt

example.txt中的文本是123456。以上一行命令是将Example.ext用myrsaCA.oem来签名，使用安全散列算法sha256，输出为example.sha256

> openssl dgst -sha256 -signature example.sha256 -verify myrsapubkey.pem example.txt

上一句是用公钥文件myraspubkey.pem来验证，结果如下，验证通过：

![](http://i4.piimg.com/aced37c78f70d3d1.png)

以上就是实验的所有部分。
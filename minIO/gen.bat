(
echo(^<service^>
echo(^<id^>MinIO^</id^>
echo(^<name^>MinIO^</name^>
echo(^<description^>MinIO is a high performance object storage server^</description^>
echo(^<executable^>minio.exe^</executable^>
echo(^<env name="MINIO_ACCESS_KEY" value="deepsight"/^>
echo(^<env name="MINIO_SECRET_KEY" value="deepsight"/^>
echo(^<arguments^>server %1^</arguments^>
echo(^<logmode^>rotate^</logmode^>
echo(^</service^>
)>%2minio-service.xml
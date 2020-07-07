#include <QCoreApplication>
#include <QAudioOutput>
#include <QAudioBuffer>
#include <QAudioDecoder>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDebug>

const QString source = "bgm.wav";
const QString sourcePos = "bgm.txt";

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QAudioDecoder decoder;
    decoder.setSourceFilename(source);
    QByteArray data, bufStart, bufLoop;
    QAudioOutput* output;
    QIODevice* device;
    QByteArray cache;
    QObject::connect(&decoder, &QAudioDecoder::bufferReady, [&]() {
        QAudioBuffer buffer = decoder.read();
        QByteArray temp((const char*)buffer.constData(), buffer.byteCount());
        data.push_back(temp);
    });
    QObject::connect(&decoder, &QAudioDecoder::finished, [&]() {
        QAudioFormat format = decoder.audioFormat();
        qDebug() << format;
        QFile info(sourcePos);
        info.open(QIODevice::ReadOnly);
        QTextStream sin(&info);
        int left, right;
        sin >> left >> right;
        info.close();
        left *= format.channelCount() * format.sampleSize() / 8;
        right *= format.channelCount() * format.sampleSize() / 8;
        bufStart = data.left(left);
        bufLoop = data.mid(left, right - left);
        output = new QAudioOutput(format);
        cache = bufStart;
        device = output->start();
        QTimer* timer = new QTimer;
        QObject::connect(timer, &QTimer::timeout, [&]() {
            int size = output->bytesFree();
            while (cache.size() < size)
                cache.append(bufLoop);
            device->write(cache.left(size));
            cache = cache.mid(size);
        });
        timer->start(50);
    });
    QObject::connect(&decoder, QOverload<QAudioDecoder::Error>::of(&QAudioDecoder::error), [&](QAudioDecoder::Error error) {
        qDebug() << error;
    });
    decoder.start();
    return a.exec();
}

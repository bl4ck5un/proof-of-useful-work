import requests as rq
import httpsig
import json
import pprint

KEY_DIR = '/home/fanz/.ias/'
cert=(KEY_DIR + '/client.crt', KEY_DIR + '/client.key')

def print_headers(headers):
    print 'HEADERS: '
    for k, v in headers.iteritems():
        print k, ': ', v

def parse_request(r):
    print '-------------BEGIN RESPONSE----------------------'
    print 'STATUS CODE: ', r.status_code
    print_headers(r.headers)

    try:
        j = r.json()
        print 'JSON: ', json.dumps(j, indent=4, separators=(',', ': '))
    except ValueError:
        pass
    print '--------------END OF RESPONSE--------------------'

intel_url = "https://test-as.sgx.trustedservices.intel.com:443/attestation/sgx/v2"

import json

quote = "AgABAG4NAAAEAAQAAAAAAPCPp6rjdFLfMtaSVrmphfAAAAAAAAAAAAAAAAAAAAAABAT//wEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABwAAAAAAAAAHAAAAAAAAACJbTq3x+zZvGzt4Jh0NUdhDhOEEAj/Dobdsy4sTpggcAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACD1xnnferKFHD2uvYqTXdDA8iZ22kCD5xw7h38CMfOngAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqpqZmZmZmdk/AQAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAqAIAAG+khyUHd+fyFZ/k+6aUJE5o+ERxZQyO8NtDzKuyDFBW8YSnMbJEdmsbcZvwt8xITFC/RmMAEbCtHcXh8QFEO4TFybNAesESwpCdcpd2fY+yBdIvfy6k8p0arnJUdEmUQE7NY9tyeYLv61jChJPbjJc8ovxDFLjNoxyEdSPq3EBcsher9bFN8UeUMLgkk82LWnv4gZ3bTGrDJ4KY1k6yvoSUz+XFvPrTCXHJrxNJBAxZih7d2OCTFqIl6af2FYbmFNb8gOjRAKmGojgK1bnKQ8oYmHwmNj9nCo6JcoQIniR+Y8tjIdwQ94VRaIHP7M8z2a75mllpVc4eJhHzNG9eY/joj9oxa4j5Bu1fftTszCqKQZ3tE2R8rKe/CJGsvrWCH5s6QaMpqD9IwiOHdmgBAACAelxIIFtuBM1N/LI5kJrsplmysJUEDRlMOLzNXafihasrC8V3LJZ4Gjax993cYPOYAhfgtCs5I+alaU+0Dk/0q+MJvchJmIGbA8OKdNXluuU6lEaRPBakh2WZmPZZl1IrMSQstOxLxbaOZnoxo/Rj6Q5SlmLPLPQtQ7tzhXzoYtM+V4TD/mAU7U2SerEqvhL78NgDkILv6w+fR7hfIG4R+cR1Oa+S69rnq0Xea8jbgjdw6GGNIuy6aJYQW95sMQKL/8743Q15xHpK94KZPd1T0BMVQlTwQgL1QdPDh3ysQtcxZIn6gcg7R1zGEJmhD/3bNYzXWxN+7ptsqHg+TrfAfEkB/T8W0DsQg7co8ZW7pD6/MODNZ/38PLIM8i+WZlkhKH7bz9OuX4oZcmITRYXJdm0MGLADdpyLNzjix902LR7ycgJDAFhJtNoZu8RUIrRRWc6SaXHmQOlRzuwVw8c2LwgPAeaorjvCLuRarhlvcXiB33F0V33a"

att_evidence_payload = {
    'isvEnclaveQuote': quote,
}
r = rq.post(intel_url + '/report', json=att_evidence_payload, cert=cert)
parse_request(r)

pubkey = open('RK_PUB.PEM', 'rb').read()
http_verifier = httpsig.Verifier(secret=pubkey, algorithm='rsa-sha256')

parse_request(r)
print r.content
print 'VERIFICATION: ', http_verifier._verify(r.content, r.headers['x-iasreport-signature'])

# happy OR not

A connected device that registers customer feedbacks from a public place where the owner prioritizes customer experience. This device enables customer to provide feedback via strategically placed booth, where customer can lodge their experienece on the basis of provided scale of experience. This will sent to server and be made available to owner and stakeholders to provide insight on customer experience. 

## working patterns
 100 entries or five minutes timeout, whichever comers first, the device will send data to the server. 
 
 ### Data
```
    {
        "time":unix-time-stamp,
        "entry":int8,
    }

```

## Device identification
A unique id will be shown while flashing the device with the code. that ID shall be the identification of the device for the entirity. 

## HTTPS connection
 data will come in this fashion:
 ```
 POST /api/data/ HTTP/1.1
Host: localhost:8000
Cache-Control: no-cache
Content-Type: application/x-www-form-urlencoded

1659011710=0&1659011715=5&1659011712=2
```
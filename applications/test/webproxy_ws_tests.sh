
test_request()
{
    echo $1
    echo $1 | websocat -1 ws://127.0.0.1:8080/messagehub/registry
    sleep 1
}

#test_request "xxx"
#test_request "{0}}"
#test_request "'xxx'"

#test_request "{'request':'xxx'}"
test_request "{'request':'list'}"
#test_request "{'request':'register'}"
#test_request "{'request':'register','entry':{'id':'mock'}}"
#test_request "{'request':'register','entry':{'id':'1b4e28ba-2fa1-11d2-883f-0016d3cca427'}}"
#test_request "{'request':'register','entry':{'id':'1b4e28ba-2fa1-11d2-883f-0016d3cca427', 'name':'mocker'}}"
#test_request "{'request':'register','entry':{'id':'1b4e28ba-2fa1-11d2-883f-0016d3cca427', 'name':'mocker', 'topic':'registry'}}"
#test_request "{'request':'register','entry':{'id':'1b4e28ba-2fa1-11d2-883f-0016d3cca427', 'name':'mocker', 'topic':'registry', 'type':'messagelink'}}"



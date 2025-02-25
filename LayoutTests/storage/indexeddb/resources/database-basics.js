if (this.importScripts) {
    importScripts('../../../fast/js/resources/js-test-pre.js');
    importScripts('shared.js');
}

description("Test the basics of IndexedDB's IDBDatabase.");

function test()
{
    removeVendorPrefixes();

    request = evalAndLog("indexedDB.open('database-basics')");
    request.onsuccess = openSuccess;
    request.onerror = unexpectedErrorCallback;
}

function openSuccess()
{
    self.db = evalAndLog("db = event.target.result");

    request = evalAndLog("db.setVersion('new version')");
    request.onsuccess = setVersionSuccess;
    request.onerror = unexpectedErrorCallback;
}

function setVersionSuccess()
{
    debug("setVersionSuccess():");
    self.trans = evalAndLog("trans = event.target.result");
    shouldBeNonNull("trans");
    trans.onabort = unexpectedAbortCallback;

    deleteAllObjectStores(db);

    debug("Testing setVersion.");
    request = evalAndLog('db.setVersion("version a")');
    request.onsuccess = setVersionAgain;
    request.onerror = unexpectedErrorCallback;
}

function setVersionAgain()
{
    request = evalAndLog('db.setVersion("version b")');
    request.onsuccess = createObjectStore;
    request.onerror = unexpectedErrorCallback;
}

function createObjectStore()
{
    shouldBeEqualToString("db.version", "version b");
    shouldBeEqualToString("db.name", "database-basics");
    shouldBe("db.objectStoreNames", "[]");
    shouldBe("db.objectStoreNames.length", "0");
    shouldBe("db.objectStoreNames.contains('')", "false");
    shouldBeNull("db.objectStoreNames[0]");
    shouldBeNull("db.objectStoreNames.item(0)");

    objectStore = evalAndLog('db.createObjectStore("test123")');
    checkObjectStore();
    commitAndContinue();
}

function checkObjectStore()
{
    shouldBe("db.objectStoreNames", "['test123']");
    shouldBe("db.objectStoreNames.length", "1");
    shouldBe("db.objectStoreNames.contains('')", "false");
    shouldBe("db.objectStoreNames.contains('test456')", "false");
    shouldBe("db.objectStoreNames.contains('test123')", "true");
}

function commitAndContinue()
{
    self.setTimeout(testSetVersionAbort, 0);
}

function testSetVersionAbort()
{
    request = evalAndLog('db.setVersion("version c")');
    request.onsuccess = createAnotherObjectStore;
    request.onerror = unexpectedErrorCallback;
}

function createAnotherObjectStore()
{
    shouldBeEqualToString("db.version", "version c");
    shouldBeEqualToString("db.name", "database-basics");
    checkObjectStore();

    objectStore = evalAndLog('db.createObjectStore("test456")');
    var setVersionTrans = evalAndLog("setVersionTrans = event.target.result");
    shouldBeNonNull("setVersionTrans");
    setVersionTrans.oncomplete = unexpectedCompleteCallback;
    setVersionTrans.onabort = checkMetadata;
    setVersionTrans.abort();
}

function checkMetadata()
{
    shouldBeEqualToString("db.version", "version b");
    checkObjectStore();
    testClose();
}

function testClose()
{
    evalAndLog("db.close()");
    debug("Now that the connection is closed, transaction creation should fail");
    evalAndExpectException("db.transaction('test123')", "IDBDatabaseException.NOT_ALLOWED_ERR");
    debug("Call twice, make sure it's harmless");
    evalAndLog("db.close()");
    finishJSTest();
}

test();
if (this.importScripts) {
    importScripts('../../../fast/js/resources/js-test-pre.js');
    importScripts('shared.js');
}

description("Test IndexedDB's IDBObjectStore.get(IDBKeyRange) method.");

function test()
{
    removeVendorPrefixes();
    openDatabase();
}

function openDatabase()
{
    result = evalAndLog("indexedDB.open('cursor-continue')");
    result.onsuccess = setVersion;
    result.onerror = unexpectedErrorCallback;
}

function setVersion()
{
    self.db = evalAndLog("db = event.target.result");

    result = evalAndLog("db.setVersion('new version')");
    result.onsuccess = deleteExisting;
    result.onerror = unexpectedErrorCallback;
}

function deleteExisting()
{
    self.trans = evalAndLog("trans = event.target.result");
    shouldBeNonNull("trans");
    trans.onabort = unexpectedAbortCallback;

    deleteAllObjectStores(db);

    self.testLength = 10;
    self.objectStore = evalAndLog("db.createObjectStore('someObjectStore')");
    self.index = evalAndLog("objectStore.createIndex('someIndex', 'x')");
    addData();
}

function addData()
{
    for (var i=0; i<self.testLength; i++) {
        evalAndLog("objectStore.add({'x': " + i + " }, " + i + ")");
    }

    evalAndLog("runObjStoreTests()");
}

function runObjStoreTests()
{
    getRangeOnlyTest("objectStore", "get", ".x", "runIndexStoreTests()");
}

function runIndexStoreTests()
{
    getRangeOnlyTest("index",  "get", ".x", "runIndexKeyTests()");
}

function runIndexKeyTests()
{
    getRangeOnlyTest("index", "getKey", "", "finishJSTest()");
}

function getRangeOnlyTest(store, method, resultPath, finish)
{
    request = evalAndLog(store + "." + method + "(IDBKeyRange.only(3))");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function()
    {
        result = event.target.result;
        shouldBe("result" + resultPath, "3");

        getRangeLowerTest(store, method, resultPath, finish);
    };
}

// A closed range with a lower bound should just return that value
function getRangeLowerTest(store, method, resultPath, finish)
{
    request = evalAndLog(store + "." + method + "(IDBKeyRange.lowerBound(5))");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function()
    {
        result = event.target.result;
        shouldBe("result" + resultPath, "5");

        getRangeLowerOpenTest(store, method, resultPath, finish);
    };
}

// An open range with a lower bound should skip the lower bound value
function getRangeLowerOpenTest(store, method, resultPath, finish)
{
    request = evalAndLog(store + "." + method + "(IDBKeyRange.lowerBound(5, true))");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function()
    {
        result = event.target.result;
        shouldBe("result" + resultPath, "6");

        getRangeUpperTest(store, method, resultPath, finish);
    };
}

// range with just upper should just return the first element
function getRangeUpperTest(store, method, resultPath, finish)
{
    request = evalAndLog(store + "." + method + "(IDBKeyRange.upperBound(7))");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function()
    {
        result = event.target.result;
        shouldBe("result" + resultPath, "0");

        getRangeUpperOpenTest(store, method, resultPath, finish);
    };
}

// range with just upper should just return the first element
function getRangeUpperOpenTest(store, method, resultPath, finish)
{
    request = evalAndLog(store + "." + method + "(IDBKeyRange.upperBound(7, true))");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function()
    {
        result = event.target.result;
        shouldBe("result" + resultPath, "0");

        getRangeLowerFractionTest(store, method, resultPath, finish);
    };
}

function getRangeLowerFractionTest(store, method, resultPath, finish)
{
    request = evalAndLog(store + "." + method + "(IDBKeyRange.lowerBound(2.5))");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function()
    {
        result = event.target.result;
        shouldBe("result" + resultPath, "3");

        getOutOfRangeTest(store, method, resultPath, finish);
    };
}

function getOutOfRangeTest(store, method, resultPath, finish)
{
    request = evalAndLog(store + "." + method + "(IDBKeyRange.lowerBound(100))");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function()
    {
        result = event.target.result;
        shouldBe("result", "undefined");

        getBadOnlyTest(store, method, resultPath, finish);
    };
}

function getBadOnlyTest(store, method, resultPath, finish)
{
    request = evalAndLog(store + "." + method + "(IDBKeyRange.only(3.3))");
    request.onerror = unexpectedErrorCallback;
    request.onsuccess = function()
    {
        result = event.target.result;
        shouldBe("result", "undefined");

        getNullTest(store, method, resultPath, finish);
    };
}

function getNullTest(store, method, resultPath, finish)
{
    evalAndExpectException(store + "." + method + "(null)", "IDBDatabaseException.DATA_ERR");
    evalAndLog(finish);
}

test();

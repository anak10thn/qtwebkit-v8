if (this.importScripts) {
    importScripts('../../../fast/js/resources/js-test-pre.js');
    importScripts('shared.js');
}

description("Test IndexedDB's KeyRange.");

function checkSingleKeyRange(value)
{
    keyRange = evalAndLog("IDBKeyRange.only(" + value + ")");
    shouldBe("keyRange.lower", "" + value);
    shouldBe("keyRange.upper", "" + value);
    shouldBeFalse("keyRange.lowerOpen");
    shouldBeFalse("keyRange.upperOpen");
}

function checkLowerBoundKeyRange(value, open)
{
    keyRange = evalAndLog("IDBKeyRange.lowerBound(" + value + "," + open + ")");
    shouldBe("keyRange.lower", "" + value);
    if (open === undefined)
        open = false;
    shouldBe("keyRange.lowerOpen", "" + open);
    shouldBeNull("keyRange.upper");
    shouldBeFalse("keyRange.upperOpen");
}

function checkUpperBoundKeyRange(value, open)
{
    keyRange = evalAndLog("IDBKeyRange.upperBound(" + value + "," + open + ")");
    shouldBe("keyRange.upper", "" + value);
    if (open === undefined)
        open = false;
    shouldBe("keyRange.upperOpen", "" + open);
    shouldBeNull("keyRange.lower");
    shouldBeFalse("keyRange.lowerOpen");
}

function checkBoundKeyRange(lower, upper, lowerOpen, upperOpen)
{
    keyRange = evalAndLog("IDBKeyRange.bound(" + lower + "," + upper + ", " + lowerOpen + ", " + upperOpen + ")");
    shouldBe("keyRange.lower", "" + lower);
    shouldBe("keyRange.upper", "" + upper);
    if (lowerOpen === undefined)
        lowerOpen = false;
    if (upperOpen === undefined)
        upperOpen = false;
    shouldBe("keyRange.lowerOpen", "" + lowerOpen);
    shouldBe("keyRange.upperOpen", "" + upperOpen);
}

function test()
{
    removeVendorPrefixes();
    shouldBeFalse("'lower' in IDBKeyRange");
    shouldBeFalse("'upper' in IDBKeyRange");
    shouldBeFalse("'lowerOpen' in IDBKeyRange");
    shouldBeFalse("'upperOpen' in IDBKeyRange");
    shouldBeTrue("'only' in IDBKeyRange");
    shouldBeTrue("'lowerBound' in IDBKeyRange");
    shouldBeTrue("'upperBound' in IDBKeyRange");
    shouldBeTrue("'bound' in IDBKeyRange");

    debug("");

    var instance = evalAndLog("instance = IDBKeyRange.only(1)");
    shouldBeTrue("'lower' in instance");
    shouldBeTrue("'upper' in instance");
    shouldBeTrue("'lowerOpen' in instance");
    shouldBeTrue("'upperOpen' in instance");
    shouldBeFalse("'only' in instance");
    shouldBeFalse("'lowerBound' in instance");
    shouldBeFalse("'upperBound' in instance");
    shouldBeFalse("'bound' in instance");

    debug("");

    checkSingleKeyRange(1);
    checkSingleKeyRange(3.14);
    checkSingleKeyRange("'a'");

    checkLowerBoundKeyRange(10, true);
    checkLowerBoundKeyRange(11, false);
    checkLowerBoundKeyRange(12);
    checkLowerBoundKeyRange(10.1, true);
    checkLowerBoundKeyRange(11.2, false);
    checkLowerBoundKeyRange(12.3);
    checkLowerBoundKeyRange("'aa'", true);
    checkLowerBoundKeyRange("'ab'", false);
    checkLowerBoundKeyRange("'ac'");

    checkUpperBoundKeyRange(20, true);
    checkUpperBoundKeyRange(21, false);
    checkUpperBoundKeyRange(22);
    checkUpperBoundKeyRange(20.2, true);
    checkUpperBoundKeyRange(21.3, false);
    checkUpperBoundKeyRange(22.4);
    checkUpperBoundKeyRange("'ba'", true);
    checkUpperBoundKeyRange("'bb'", false);
    checkUpperBoundKeyRange("'bc'");

    checkBoundKeyRange(30, 40);
    checkBoundKeyRange(31, 41, false, false);
    checkBoundKeyRange(32, 42, false, true);
    checkBoundKeyRange(33, 43, true, false);
    checkBoundKeyRange(34, 44, true, true);

    checkBoundKeyRange(30.1, 40.2);
    checkBoundKeyRange(31.3, 41.4, false, false);
    checkBoundKeyRange(32.5, 42.6, false, true);
    checkBoundKeyRange(33.7, 43.8, true, false);
    checkBoundKeyRange(34.9, 44.0, true, true);

    checkBoundKeyRange("'aaa'", "'aba'", false, false);
    checkBoundKeyRange("'aab'", "'abb'");
    checkBoundKeyRange("'aac'", "'abc'", false, false);
    checkBoundKeyRange("'aad'", "'abd'", false, true);
    checkBoundKeyRange("'aae'", "'abe'", true, false);
    checkBoundKeyRange("'aaf'", "'abf'", true, true);

    debug("Passing an invalid key into only({})");
    evalAndExpectException("IDBKeyRange.only({})", "IDBDatabaseException.DATA_ERR");

    debug("Passing an invalid key into upperBound({})");
    evalAndExpectException("IDBKeyRange.upperBound({})", "IDBDatabaseException.DATA_ERR");

    debug("Passing an invalid key into lowerBound({})");
    evalAndExpectException("IDBKeyRange.lowerBound({})", "IDBDatabaseException.DATA_ERR");

    debug("Passing an invalid key into bound(null, {})");
    evalAndExpectException("IDBKeyRange.bound(null, {})", "IDBDatabaseException.DATA_ERR");

    debug("Passing an invalid key into bound({},null)");
    evalAndExpectException("IDBKeyRange.bound({}, null)", "IDBDatabaseException.DATA_ERR");

    debug("Passing an invalid key into bound({}, {})");
    evalAndExpectException("IDBKeyRange.bound({}, {})", "IDBDatabaseException.DATA_ERR");

    debug("Lower key greater than higher key, bound(4, 3)");
    evalAndExpectException("IDBKeyRange.bound(4, 3)", "IDBDatabaseException.DATA_ERR");

    debug("Equal keys, either of the bounds is open, bound(4, 4, true, false)");
    evalAndExpectException("IDBKeyRange.bound(4, 4, true, false)", "IDBDatabaseException.DATA_ERR");

    debug("Equal keys, either of the bounds is open, bound(4, 4, false, true)");
    evalAndExpectException("IDBKeyRange.bound(4, 4, false, true)", "IDBDatabaseException.DATA_ERR");

    debug("Equal keys, either of the bounds is open, bound(4, 4, true, true)");
    evalAndExpectException("IDBKeyRange.bound(4, 4, true, true)", "IDBDatabaseException.DATA_ERR");

    debug("Equal keys, none of the bounds is open, bound(4, 4, false, false)");
    IDBKeyRange.bound(4, 4, false, false);
}

test();

finishJSTest();
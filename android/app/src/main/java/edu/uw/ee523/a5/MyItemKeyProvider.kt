package edu.uw.ee523.btdemo

import androidx.recyclerview.selection.ItemKeyProvider

class MyItemKeyProvider(private val adapter: BluetoothDeviceListAdapter) : ItemKeyProvider<String>(
    SCOPE_CACHED
)
{
    override fun getKey(position: Int): String? =
        adapter.currentList[position].address
    override fun getPosition(key: String): Int =
        adapter.currentList.indexOfFirst {it.address == key}
}